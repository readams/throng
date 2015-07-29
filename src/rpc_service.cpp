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
using std::chrono::steady_clock;
using boost::system::error_code;
namespace ba = boost::asio;

LOGGER("rpc");

static const singleton_task::duration_type MANAGE_CONNS_INTERVAL(3000);

rpc_service::rpc_service(ctx_internal& ctx_,
                         handler_factory_t& handler_factory_)
    : ctx(ctx_), handler_factory(handler_factory_),
      running(false), next_conn_id(0), acceptor(ctx.get_io_service()),
      manage_conns_task(ctx.get_io_service(),
                     std::bind(&rpc_service::manage_conns, this)) {

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

    // XXX TODO - set port/ip from config
    server_endpoint = { ba::ip::tcp::v4(), ctx.get_local_seed().second };
    acceptor.open(server_endpoint.protocol());
    acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(server_endpoint);
    acceptor.listen();

    accept();
    manage_conns_task.schedule();
}

rpc_connection_p rpc_service::new_conn(const node_id& remote_node_id) {
    auto handle_stop = [this, remote_node_id](const rpc_connection_p& conn) {
        std::unique_lock<std::mutex> guard(conn_mutex);
        connections.erase(conn->get_conn_id());

        const node_id* eff_rid = &remote_node_id;
        if (remote_node_id.size() == 0)
            eff_rid = &conn->get_handler().get_remote_node_id();

        auto it = node_connections.find(*eff_rid);
        if (it != node_connections.end() && it->second.conn.get() == conn.get()) {
            it->second.conn.reset();
            it->second.ready = false;
        }
        if (running.load() && conn->get_conn_id() == bootstrap_conn_id)
            bootstrap();
        manage_conns_task.schedule();
    };

    auto handle_ready = [this, remote_node_id](const rpc_connection_p& conn) {
        std::unique_lock<std::mutex> guard(conn_mutex);
        seed_iter = seeds.begin();
        const node_id& rid = conn->get_handler().get_remote_node_id();
        if (remote_node_id.size() != 0 && rid != remote_node_id) {
            LOG(ERROR) << ctx.get_local_node_id() << ":" << conn->get_conn_id()
                       << " Remote node ID " << rid << " unexpected; should be "
                       << remote_node_id;
            conn->stop();
        } else if (rid.size() != 0) {
            auto it = node_connections.find(rid);
            if (it != node_connections.end()) {
                if (conn.get() != it->second.conn.get()) {
                    LOG(ERROR) << ctx.get_local_node_id() << ":"
                               << conn->get_conn_id()
                               << " Removing old connection from " << it->first;
                    it->second.conn->stop();
                    node_connections.erase(it);
                    it = node_connections.end();
                } else {
                    it->second.ready = true;
                    for (auto& a : it->second.actions)
                        a.second(conn);
                    it->second.actions.clear();
                }
            }
            if (it == node_connections.end()) {
                node_connections.emplace(rid,node_conn{conn, true,
                            steady_clock::now()});
            }
        }
    };

    rpc_handler_p handler = handler_factory();
    handler->add_ready_listener(handle_ready);
    auto conn = make_shared<rpc_connection>(ctx, next_conn_id++,
                                            std::move(handler),
                                            std::move(handle_stop));
    return conn;
}

void rpc_service::bootstrap() {
    if (seed_iter == seeds.end()) {
        if (seeds.size() > 0) {
            LOG(ERROR) << "Could not connect to any seed for bootstrapping";
        }
        return;
    }
    rpc_connection_p conn = new_conn(node_id{});
    bootstrap_conn_id = conn->get_conn_id();
    connections.insert({ conn->get_conn_id(), conn });
    conn->start(seed_iter->first, seed_iter->second);
    seed_iter++;
}

void rpc_service::accept() {
    rpc_connection_p conn = new_conn(node_id{});
    auto handle_accept = [this, conn](const error_code& ec) {
        if (!ec) {
            {
                std::unique_lock<std::mutex> guard(conn_mutex);
                connections.insert({ conn->get_conn_id(), conn });
            }
            conn->start();
        }
        if (running) accept();
    };
    acceptor.async_accept(conn->get_socket(), handle_accept);
}

void rpc_service::stop() {
    if (!running.load()) return;

    running.store(false);

    manage_conns_task.cancel();

    std::vector<rpc_connection_p> to_close;
    std::unique_lock<std::mutex> guard(conn_mutex);
    acceptor.close();
    for (auto& c : connections)
        to_close.push_back(c.second);
    for (auto& c : to_close)
        c->stop();
}

bool rpc_service::is_ready(node_id& node_id) {
    std::unique_lock<std::mutex> guard(conn_mutex);
    auto it = node_connections.find(node_id);
    if (it == node_connections.end()) return false;
    return it->second.ready;
}

// must hold conn_mutex when calling
void rpc_service::connect_to_node(const node_id& id) {
    rpc_connection_p* conn = nullptr;

    auto it = node_connections.find(id);
    if (it == node_connections.end()) {
        auto r = node_connections.emplace(id,
                                          node_conn{nullptr, false,
                                                    steady_clock::now()});
        conn = &r.first->second.conn;
    } else {
        it->second.last_required = steady_clock::now();
        if (it->second.conn) {
            // already connected
            return;
        }
        conn = &it->second.conn;
    }

    message::node_id nid;
    for (auto i : id)
        nid.add_id(i);
    versioned<message::node> node = node_client->get(nid);
    if (!node) {
        // need to find the node
        return;
    }

    *conn = new_conn(id);
    connections.insert({ (*conn)->get_conn_id(), *conn });
    (*conn)->start(node.get().hostname(), node.get().port());
}

void rpc_service::connect_to_neigh(const message::neighborhood& neigh) {
    for (auto& master : neigh.masters()) {
        node_id master_id;
        for (auto i : master.id())
            master_id.push_back(i);

        if (ctx.is_master() &&
            master_id >= ctx.get_local_node_id()) continue;

        std::unique_lock<std::mutex> guard(conn_mutex);
        connect_to_node(master_id);
    }
}

void rpc_service::manage_conns() {
    if (!running.load()) return;

    auto cc = ctx.get_cluster_config();
    if (cc) {
        if (ctx.is_master()) {
            // Connect to all master nodes for all neighborhoods
            for (auto& nv : cc->get_neighborhoods()) {
                connect_to_neigh(*nv.second);
            }
        } else {
            // Connect only to master nodes in the local neighborhood
            node_id local_nid = ctx.get_local_node_id();
            local_nid.pop_back();
            auto neigh = cc->get_neighborhood(local_nid);
            if (neigh)
                connect_to_neigh(*neigh.get());
        }
    }

    time_point now = steady_clock::now();
    {
        std::unique_lock<std::mutex> guard(conn_mutex);
        for (auto& v : node_connections) {
            if (v.second.actions.size() == 0 &&
                now > v.second.last_required + (MANAGE_CONNS_INTERVAL*2)) {
                LOG(DEBUG) << ctx.get_local_node_id() << ":"
                           << v.second.conn->get_conn_id()
                           << " Node connection no longer required";
                v.second.conn->stop();
            }
        }
    }

    manage_conns_task.schedule(MANAGE_CONNS_INTERVAL);
}

void rpc_service::dispatch_node_action(const node_id& node_id,
                                       const std::string& action_key,
                                       node_action_t action) {
    std::unique_lock<std::mutex> guard(conn_mutex);
    connect_to_node(node_id);
    auto it = node_connections.find(node_id);
    if (it->second.ready)
        ctx.get_io_service().dispatch(std::bind(action, it->second.conn));
    else if (it->second.actions.find(action_key) == it->second.actions.end())
        it->second.actions.emplace(action_key, action);
}

} /* namespace internal */
} /* namespace throng */
