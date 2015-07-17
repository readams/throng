/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for ctx class.
 *
 * Copyright (c) 2015 Cisco Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ctx_internal.h"
#include "cluster_config.h"
#include "logger.h"
#include "throng/store_client.h"
#include "throng/serializer_protobuf.h"
#include "throng_messages.pb.h"
#include "store_registry.h"
#include "rpc_service.h"
#include "rpc_handler_node.h"

#include <vector>
#include <utility>
#include <mutex>
#include <thread>

//#include <leveldb/db.h>

THRONG_PROTOBUF_SERIALIZER(throng::message::node)
THRONG_PROTOBUF_SERIALIZER(throng::message::neighborhood)

namespace throng {
namespace internal {

using std::string;
using std::vector;
using std::unique_ptr;
using std::pair;
using std::make_shared;
using std::shared_ptr;
using boost::asio::io_service;
using throng::message::node;
using throng::message::neighborhood;

LOGGER("core");

static const string NODE_STORE = "__sys_node_store";
static const string NEIGH_STORE = "__sys_neigh_store";

typedef store_client<string, node> node_client_t;
typedef store_client<string, neighborhood> neigh_client_t;

class ctx_impl : public ctx_internal {
public:
    ctx_impl(string db_path);
    virtual ~ctx_impl();

    // ***
    // ctx
    // ***

    virtual void configure_local(node_id node_id,
                                 std::string hostname, uint16_t port,
                                 bool master_eligible = true);
    virtual void add_seed(std::string hostname, uint16_t port) override;
    virtual void register_store(const std::string& name) override;
    virtual void register_store(const std::string& name,
                                const store_config& config) override;
    virtual void start(size_t worker_pool_size = 3) override;
    virtual void stop() override;
    virtual node_id get_local_node_id() override;
    virtual void add_raw_listener(const std::string& store_name,
                                  raw_listener_type listener) override;
    virtual store<std::string,std::string>&
    get_raw_store(const std::string& name) override;

    // ************
    // ctx_internal
    // ************

    virtual boost::asio::io_service& get_io_service() override;
    virtual rpc_service& get_rpc_service() override;
    virtual seed_type& get_local_seed() override;
    virtual std::shared_ptr<cluster_config> get_cluster_config() override;

private:
    internal::store_registry registry;
    volatile bool running = false;
    io_service io;
    unique_ptr<io_service::work> work;
    std::vector<std::thread> workers;

    rpc_service::handler_factory_type handler_factory;
    rpc_service rpc;

    unique_ptr<node_client_t> node_client;
    unique_ptr<neigh_client_t> neigh_client;

    std::mutex config_mutex;
    shared_ptr<cluster_config> config;

    node_id local_node_id;
    rpc_service::seed_type local_seed;
    bool master_eligible = true;
    vector<seed_type> seeds;
    //unique_ptr<leveldb::DB> systemDb;
};

io_service& ctx_impl::get_io_service() {
    return io;
}

rpc_service& ctx_impl::get_rpc_service() {
    return rpc;
}

ctx_impl::ctx_impl(string db_path)
    : registry(*this, std::move(db_path)),
      handler_factory([this]() { return make_shared<rpc_handler_node>(*this); }),
      rpc(*this, handler_factory) {

    store_config node_store_conf;
    node_store_conf.persistent = true;
    register_store(NODE_STORE, node_store_conf);
    node_client = node_client_t::new_store_client(*this, NODE_STORE);

    store_config neigh_store_conf;
    neigh_store_conf.persistent = true;
    register_store(NEIGH_STORE, neigh_store_conf);
    neigh_client = neigh_client_t::new_store_client(*this, NEIGH_STORE);
}

ctx_impl::~ctx_impl() {
    stop();
}

void ctx_impl::configure_local(node_id node_id, string hostname, uint16_t port,
                               bool master_eligible) {
    std::unique_lock<std::mutex> guard(config_mutex);
    local_seed = {std::move(hostname), port};
    local_node_id = std::move(node_id);
    master_eligible = std::move(master_eligible);
}

rpc_service::seed_type& ctx_impl::get_local_seed() {
    std::unique_lock<std::mutex> guard(config_mutex);
    return local_seed;
}

std::shared_ptr<cluster_config> ctx_impl::get_cluster_config() {
    std::unique_lock<std::mutex> guard(config_mutex);
    return config;
}

void ctx_impl::add_seed(std::string hostname, uint16_t port) {
    std::unique_lock<std::mutex> guard(config_mutex);
    seeds.emplace_back(std::move(hostname), port);
}

void ctx_impl::register_store(const std::string& name) {
    register_store(name, store_config {});
}

void ctx_impl::register_store(const std::string& name,
                         const store_config& config) {
    registry.register_store(name, config);
}

void ctx_impl::start(size_t worker_pool_size) {
    if (running) return;
    running = true;

    LOG(INFO) << "Starting distributed policy engine";

    work.reset(new io_service::work(io));

    for (size_t i = 0; i < worker_pool_size; i++) {
        workers.emplace_back([this]() {
                while (this->running) {
                    try {
                        io.run();
                    } catch (const std::exception& e) {
                        LOG(ERROR) << "Exception while processing I/O: "
                                   << e.what();
                    } catch (...) {
                        LOG(ERROR) << "Unknown error while processing I/O";
                    }
                }
            });
    }

    rpc.set_seeds(seeds);
    rpc.start();

//    leveldb::Options options;
//    options.create_if_missing = true;
//    leveldb::DB* dbPtr = nullptr;
//    path systemDbPath = dbPath;
//    systemDbPath /= "__system__";
//    leveldb::Status status = leveldb::DB::Open(options,
//                                               systemDbPath.string(),
//                                               &dbPtr);
//    if (!status.ok()) {
//        throw std::runtime_error(string("Could not create/open database: ") +
//                                 status.ToString());
//    } else {
//        systemDb.reset(dbPtr);
//        dbPtr = nullptr;
//    }
}

void ctx_impl::stop() {
    if (!running) return;
    running = false;

    LOG(INFO) << "Stopping distributed policy engine";

    rpc.stop();
    work.reset();

    for (auto& t : workers)
        t.join();
    workers.clear();
}

store<string,string>& ctx_impl::get_raw_store(const string& name) {
    return registry.get(name);
}

void ctx_impl::add_raw_listener(const std::string& store_name,
                                raw_listener_type listener) {
    registry.get(store_name).add_listener(listener);
}

node_id ctx_impl::get_local_node_id() {
    std::unique_lock<std::mutex> guard(config_mutex);
    return local_node_id;
}

} /* namespace internal */

std::unique_ptr<ctx> ctx::new_ctx(std::string db_path) {
    return std::unique_ptr<ctx>(new internal::ctx_impl(db_path));
}

} /* namespace throng */
