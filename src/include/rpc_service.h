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
#include "singleton_task.h"
#include "throng_messages.pb.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <mutex>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <utility>
#include <memory>

namespace throng {
namespace internal {

class rpc_connection;
typedef std::shared_ptr<rpc_connection> rpc_connection_p;
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
    typedef std::function<std::shared_ptr<rpc_handler>()> handler_factory_t;

    /**
     * Create a new RPC service
     *
     * @param ctx the internal context
     * @param handler_factory a factory that will generate RPC
     * connection handlers for new RPC node connections.
     */
    rpc_service(ctx_internal& ctx, handler_factory_t& handler_factory);

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
    typedef ctx_internal::seed_t seed_t;

    /**
     * Bootstrap the cluster by connecting to the provided list of seeds
     *
     * @param seeds_ the list of seeds to connect to
     */
    void set_seeds(std::vector<seed_t> seeds_) { seeds = std::move(seeds_); }

    /**
     * Check if there is a ready connection for the requested node
     *
     * @param node_id the node ID for the node
     * @return true if there is an active, ready connection for the
     * given node
     */
    bool is_ready(node_id& node_id);

    /**
     * An action to perform on a node connection once the node is
     * ready.
     */
    typedef std::function<void(const rpc_connection_p& conn)> node_action_t;

    /**
     * Run the given action on the specified node.  If the node is
     * ready immediately, the action will run immediately but
     * asynchronously.
     *
     * @param node_id the node ID on which the action should run
     * @param action_key a key for the action.  If an action already
     * is pending for the given key, then the new action will not be
     * added.
     * @param action the action to run
     */
    void dispatch_node_action(const node_id& node_id,
                              const std::string& action_key,
                              node_action_t action);

private:
    ctx_internal& ctx;
    handler_factory_t& handler_factory;

    std::unique_ptr<node_client_t> node_client;
    std::unique_ptr<neigh_client_t> neigh_client;

    std::vector<seed_t> seeds;
    decltype(seeds)::iterator seed_iter;
    uint64_t bootstrap_conn_id = std::numeric_limits<uint64_t>::max();

    std::atomic_bool running;
    std::atomic_uint_fast64_t next_conn_id;

    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::endpoint server_endpoint;

    singleton_task manage_conns_task;

    std::mutex conn_mutex;
    std::unordered_map<uint64_t, rpc_connection_p> connections;

    typedef std::chrono::steady_clock::time_point time_point;

    struct node_conn {
        rpc_connection_p conn;
        bool ready;
        time_point last_required;
        std::unordered_map<std::string, node_action_t> actions;
    };
    std::unordered_map<node_id, node_conn> node_connections;

    void accept();
    rpc_connection_p new_conn(const node_id& remote_node_id);
    void bootstrap();
    void manage_conns();
    void connect_to_neigh(const message::neighborhood& neigh);
    void connect_to_node(const node_id& id);
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_RPC_SERVICE_H */
