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

namespace throng {
namespace internal {

using std::make_shared;
using boost::system::error_code;
namespace ba = boost::asio;

rpc_service::rpc_service(ctx_internal& ctx_,
                         handler_factory_type& handler_factory_)
    : ctx(ctx_), handler_factory(handler_factory_),
      strand(ctx.get_io_service()), acceptor(ctx.get_io_service()) {

}

void rpc_service::start() {
    if (running.load()) return;
    running.store(true);

    seed_iter = seeds.begin();

    // XXX TODO - set port/ip from config
    server_endpoint = { ba::ip::tcp::v4(), ctx.get_local_seed().second };
    acceptor.open(server_endpoint.protocol());
    acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(server_endpoint);
    acceptor.listen();

    strand.dispatch([this]() { accept(); bootstrap(); });
}

rpc_service::rpc_connection_p rpc_service::new_conn() {
    auto handle_stop = std::bind(&rpc_service::connection_stopped, this,
                                 std::placeholders::_1);
    auto conn = make_shared<rpc_connection>(ctx, next_conn_id++,
                                            handler_factory(), handle_stop);
    return conn;
}

void rpc_service::bootstrap() {
    if (seed_iter == seeds.end()) {
        if (seeds.size() > 0) {
            LOG(ERROR) << "Could not connect to any seed for bootstrapping";
        }
        return;
    }
    auto conn = new_conn();
    bootstrap_conn_id = conn->get_conn_id();
    connections.insert({ conn->get_conn_id(), conn });
    conn->start(seed_iter->first, seed_iter->second);
    seed_iter++;
}

void rpc_service::connection_ready(const rpc_connection_p& conn) {

}

void rpc_service::connection_stopped(const rpc_connection_p& conn) {
    strand.post([this, conn]() {
            connections.erase(conn->get_conn_id());
            node_connections.erase(conn->get_handler().get_remote_node_id());
            if (running.load() && conn->get_conn_id() == bootstrap_conn_id) {
                bootstrap();
            }
        });
}

void rpc_service::accept() {
    auto conn = new_conn();
    auto handle_accept = [this, conn](const error_code& ec) {
        if (!ec) {
            connections.insert({ conn->get_conn_id(), conn });
            conn->start();
        }
        if (running) accept();
    };
    acceptor.async_accept(conn->get_socket(), strand.wrap(handle_accept));
}

void rpc_service::stop() {
    if (running.load()) {
        running.store(false);

        strand.dispatch([this]() {
                acceptor.close();
                std::vector<rpc_connection_p> to_close;
                for (auto& c : connections)
                    to_close.push_back(c.second);
                for (auto& c : to_close)
                    c->stop();
            });
    }
}

} /* namespace internal */
} /* namespace throng */
