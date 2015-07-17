/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file rpc_service.h
 * @brief Interface definition file for rpc_service
 */
/* Copyright (c) 2015 Cisco Systems, Inc.
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

#pragma once
#ifndef THRONG_RPC_SERVICE_H
#define THRONG_RPC_SERVICE_H

#include "ctx_internal.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <mutex>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <utility>

namespace throng {
namespace internal {

class rpc_connection;
class rpc_handler;

/**
 * Maintains connections to other nodes in the cluster as required,
 * and allows sending messages.
 */
class rpc_service {
public:
    /**
     * A handler factory creates a new handler for RPC connections
     * when then are created.
     */
    typedef std::function<std::shared_ptr<rpc_handler>()> handler_factory_type;

    /**
     * Create a new RPC service
     *
     * @param ctx the internal context
     * @param handler_factory a factory that will generate RPC
     * connection handlers for new RPC node connections.
     */
    rpc_service(ctx_internal& ctx, handler_factory_type& handler_factory);

    /**
     * Start the RPC service
     */
    void start();

    /**
     * Disconnect all connections and stop the RPC service
     */
    void stop();

    /**
     * A seed for bootstrapping the cluster
     */
    typedef ctx_internal::seed_type seed_type;

    /**
     * Bootstrap the cluster by connecting to the provided list of seeds
     *
     * @param seeds_ the list of seeds to connect to
     */
    void set_seeds(std::vector<seed_type> seeds_) { seeds = std::move(seeds_); }

    /**
     * Check if there is a ready connection for the requested node
     *
     * @param node_id the node ID for the node
     * @return true if there is an active, ready connection for the
     * given node
     */
    bool is_ready(node_id& node_id);

private:
    ctx_internal& ctx;
    handler_factory_type& handler_factory;
    std::vector<seed_type> seeds;
    decltype(seeds)::iterator seed_iter;
    uint64_t bootstrap_conn_id = std::numeric_limits<uint64_t>::max();

    std::atomic_bool running;
    std::atomic_uint_fast64_t next_conn_id;

    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::endpoint server_endpoint;

    typedef std::shared_ptr<rpc_connection> rpc_connection_p;

    std::recursive_mutex conn_mutex;
    std::unordered_map<uint64_t, rpc_connection_p> connections;
    std::unordered_map<node_id, rpc_connection_p> node_connections;

    void accept();
    void connection_stopped(const rpc_connection_p& conn);
    void connection_ready(const rpc_connection_p& conn);
    rpc_connection_p new_conn();
    void bootstrap();
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_RPC_SERVICE_H */
