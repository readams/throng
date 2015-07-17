/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file rpc_handler.h
 * @brief Interface definition file for rpc_handler
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
#ifndef THRONG_RPC_HANDLER_H
#define THRONG_RPC_HANDLER_H

#include "ctx_internal.h"
#include "throng_messages.pb.h"

namespace throng {
namespace internal {

class rpc_connection;

/**
 * Handles RPC messages
 */
class rpc_handler {
public:
    /**
     * Create a new RPC handler
     */
    rpc_handler(ctx_internal& ctx);
    virtual ~rpc_handler() {};

    // ****************
    // Connection state
    // ****************

    /**
     * Get the remote node ID for this connection
     *
     * @return the remote node ID as reported in the hello message
     */
    const node_id& get_remote_node_id() { return remote_node_id; }

    // **************
    // Event handlers
    // **************

    /**
     * Handle the RPC message
     *
     * @param connection the connection that recieved the message
     * @param message the message to handle
     * @return true if successful
     */
    virtual bool handle_message(rpc_connection& connection,
                                message::rpc_message& message);

    /**
     * Handle the initial connection event
     *
     * @param connection the new connection
     * @return true if successful
     */
    virtual bool handle_connect(rpc_connection& connection);

    /**
     * Handle the ready event which is called after the handshake
     * succeeds.
     *
     * @param connection the connection
     * @return true if successful
     */
    virtual bool handle_ready(rpc_connection& connection);

    /**
     * Handler for a callback to be called by the handler on
     * connection ready event by default ready handler.
     */
    typedef std::function<void(const std::shared_ptr<rpc_connection>& conn)>
    ready_listener_type;

    /**
     * Add a ready listener callback to be called by the default ready
     * handler.
     *
     * @param listener the listener to call
     */
    virtual void add_ready_listener(ready_listener_type listener);

    // **********************************
    // Generic message handling utilities
    // **********************************

    /**
     * Handle an unsupported/unknown message type
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the unsupported message
     * @param method the unsupported method
     */
    virtual void handle_unsupported(rpc_connection& connection,
                                    uint64_t xid,
                                    message::method method);

    /**
     * Handle an error generically
     *
     * @param connection the connection for the error
     * @param method The method associated with the error
     * @param status_code the status code for the error
     * @param status_message the status message
     */
    virtual void handle_error(rpc_connection& connection,
                              message::method method,
                              message::status status_code,
                              const std::string& status_message);

    // ****************
    // Message handlers
    // ****************

    /**
     * Handle the hello request message
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param message the message to handle
     * @return true if default handling successful
     */
    virtual bool handle_req_hello(rpc_connection& connection,
                                  uint64_t xid,
                                  const message::req_hello& message);

    /**
     * Handle the hello reply message
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param message the message to handle
     * @return true if default handling successful
     */
    virtual bool handle_rep_hello(rpc_connection& connection,
                                  uint64_t xid,
                                  const message::rep_hello& message);

    /**
     * Handle the hello error reply
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param status_code the status code
     * @param status_message the status message
     */
    virtual void handle_error_hello(rpc_connection& connection,
                                    uint64_t xid,
                                    message::status status_code,
                                    const std::string& status_message);

    /**
     * Handle the get request message
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param message the message to handle
     */
    virtual void handle_req_get(rpc_connection& connection,
                                uint64_t xid,
                                const message::req_get& message);

    /**
     * Handle the get reply message
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param message the message to handle
     */
    virtual void handle_rep_get(rpc_connection& connection,
                                uint64_t xid,
                                const message::rep_get& message);

    /**
     * Handle the get error reply
     *
     * @param connection the connection that recieved the message
     * @param xid the transaction ID for the message
     * @param status_code the status code
     * @param status_message the status message
     */
    virtual void handle_error_get(rpc_connection& connection,
                                  uint64_t xid,
                                  message::status status_code,
                                  const std::string& status_message);

    std::ostream& print(std::ostream& out);

protected:
    enum class conn_state {
        NEW,
        READY
    };

    ctx_internal& ctx;
    node_id remote_node_id;
    conn_state state = conn_state::NEW;

    std::vector<ready_listener_type> ready_listeners;
};

typedef std::shared_ptr<rpc_handler> rpc_handler_p;

#define HTAG ctx.get_local_node_id() << "->" << remote_node_id << " "

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_RPC_HANDLER_H */
