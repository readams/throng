/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for rpc_service class.
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

#include "rpc_service.h"
#include "rpc_connection.h"
#include "logger.h"
#include "cluster_config.h"

namespace throng {
namespace internal {

using std::make_shared;
using boost::system::error_code;
namespace ba = boost::asio;

LOGGER("rpc");

rpc_service::rpc_service(ctx_internal& ctx_,
                         handler_factory_type& handler_factory_)
    : ctx(ctx_), handler_factory(handler_factory_),
      running(false), next_conn_id(0), acceptor(ctx.get_io_service()) {

    store_config node_store_conf;
    node_store_conf.persistent = true;
    ctx.register_store(NODE_STORE, node_store_conf);

    store_config neigh_store_conf;
    neigh_store_conf.persistent = true;
    ctx.register_store(NEIGH_STORE, neigh_store_conf);
}

void rpc_service::start() {
    if (running.load()) return;
    running.store(true);

    node_client = node_client_t::new_store_client(ctx, NODE_STORE);
    neigh_client = neigh_client_t::new_store_client(ctx, NEIGH_STORE);

    seed_iter = seeds.begin();

    // XXX TODO - set port/ip from config
    server_endpoint = { ba::ip::tcp::v4(), ctx.get_local_seed().second };
    acceptor.open(server_endpoint.protocol());
    acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(server_endpoint);
    acceptor.listen();

    accept();
    bootstrap();
}

rpc_service::rpc_connection_p rpc_service::new_conn() {
    auto handle_stop = std::bind(&rpc_service::connection_stopped, this,
                                 std::placeholders::_1);
    rpc_handler_p handler = handler_factory();
    handler->add_ready_listener(std::bind(&rpc_service::connection_ready, this,
                                          std::placeholders::_1));
    auto conn = make_shared<rpc_connection>(ctx, next_conn_id++,
                                            std::move(handler),
                                            std::move(handle_stop));
    return conn;
}

void rpc_service::bootstrap() {
    std::unique_lock<std::recursive_mutex> guard(conn_mutex);
    if (seed_iter == seeds.end()) {
        if (seeds.size() > 0) {
            LOG(ERROR) << "Could not connect to any seed for bootstrapping";
        }
        return;
    }
    rpc_connection_p conn = new_conn();
    bootstrap_conn_id = conn->get_conn_id();
    connections.insert({ conn->get_conn_id(), conn });
    conn->start(seed_iter->first, seed_iter->second);
    seed_iter++;
}

void rpc_service::connection_ready(const rpc_connection_p& conn) {
    std::unique_lock<std::recursive_mutex> guard(conn_mutex);
    seed_iter = seeds.begin();
    node_id rid = conn->get_handler().get_remote_node_id();
    if (rid != node_id{}) {
        auto it = node_connections.find(rid);
        if (it != node_connections.end()) {
            LOG(ERROR) << "Removing old connection from " << it->first;
            it->second->stop();
            node_connections.erase(it);
        }
        node_connections.emplace(rid, conn);
    }
}

void rpc_service::connection_stopped(const rpc_connection_p& conn) {
    std::unique_lock<std::recursive_mutex> guard(conn_mutex);
    connections.erase(conn->get_conn_id());
    auto it = node_connections.find(conn->get_handler().get_remote_node_id());
    if (it != node_connections.end() && it->second.get() == conn.get())
        node_connections.erase(it);
    if (running.load() && conn->get_conn_id() == bootstrap_conn_id) {
        bootstrap();
    }
}

void rpc_service::accept() {
    rpc_connection_p conn = new_conn();
    auto handle_accept = [this, conn](const error_code& ec) {
        if (!ec) {
            {
                std::unique_lock<std::recursive_mutex> guard(conn_mutex);
                connections.insert({ conn->get_conn_id(), conn });
            }
            conn->start();
        }
        if (running) accept();
    };
    acceptor.async_accept(conn->get_socket(), handle_accept);
}

void rpc_service::stop() {
    if (running.load()) {
        running.store(false);

        std::vector<rpc_connection_p> to_close;
        std::unique_lock<std::recursive_mutex> guard(conn_mutex);
        acceptor.close();
        for (auto& c : connections)
            to_close.push_back(c.second);
        for (auto& c : to_close)
            c->stop();
    }
}

bool rpc_service::is_ready(node_id& node_id) {
    std::unique_lock<std::recursive_mutex> guard(conn_mutex);
    return node_connections.find(node_id) != node_connections.end();
}

} /* namespace internal */
} /* namespace throng */
