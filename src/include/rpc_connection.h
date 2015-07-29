/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file rpc_service.h
 * @brief Interface definition file for rpc_serice
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
#ifndef THRONG_RPC_CONNECTION_H
#define THRONG_RPC_CONNECTION_H

#include "ctx_internal.h"
#include "rpc_handler.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include <memory>
#include <atomic>

namespace throng {
namespace internal {

/**
 * A connection to a remote node
 */
class rpc_connection
    : public std::enable_shared_from_this<rpc_connection> {
public:
    /**
     * Handler for framework callback on connection stop
     */
    typedef std::function<void(const std::shared_ptr<rpc_connection>& conn)>
    stop_handler_type;

    /**
     * Create a new RPC connection using the specified handler
     *
     * @param ctx the internal context
     * @param conn_id the ID for this connection
     * @param handler the handler for this connection
     * @param stop_handler a function to be called when the
     * connection is stopped
     */
    rpc_connection(ctx_internal& ctx,
                   uint64_t conn_id,
                   std::shared_ptr<rpc_handler> handler,
                   stop_handler_type stop_handler = stop_handler_type{});
    rpc_connection(const rpc_connection&) = delete;
    rpc_connection& operator=(const rpc_connection&) = delete;

    /**
     * Get the socket for this connection
     */
    boost::asio::ip::tcp::socket& get_socket() { return socket; }

    /**
     * Get the connection ID for this connection
     *
     * @return the connection ID
     */
    uint64_t get_conn_id() { return conn_id; }

    /**
     * Get the handler for this connection
     *
     * @return the handler
     */
    rpc_handler& get_handler() { return *handler; }

    /**
     * Start the connection as a client by connecting to the given
     * hostname and port
     *
     * @param hostname the hostname to connect to
     * @param port the port number to connect to
     */
    void start(std::string hostname, uint16_t port);

    /**
     * Start the connection as a server after accept.
     */
    void start();

    /**
     * Stop the connection
     */
    void stop();

    /**
     * Send a message over this connection.  If the transaction ID is
     * not already set, one will be allocated from a sequential
     * counter for the connection.
     *
     * @param m the message to send
     */
    void send_message(message::rpc_message& m);

    /**
     * Send an error reply message
     *
     * @param xid the transaction ID for the reply
     * @param method the method for the error
     * @param status_code the status code to send for the message
     * @param status_message the status message to send
     */
    void send_error_message(uint64_t xid,
                            message::method method,
                            message::status status_code,
                            const std::string& status_message);

private:
    ctx_internal& ctx;
    uint64_t conn_id;
    std::shared_ptr<rpc_handler> handler;
    stop_handler_type stop_handler;

    std::string remote_hostname;
    uint16_t remote_port;

    boost::asio::io_service::strand strand;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::resolver resolver;

    std::atomic_uint_fast64_t next_xid;

    typedef std::chrono::steady_clock::time_point time_point;
    time_point last_read;
    time_point last_write;
    std::unique_ptr<boost::asio::steady_timer> timeout;

    std::vector<uint8_t> buffer;

    void read_message();
};

typedef std::shared_ptr<rpc_connection> rpc_connection_p;

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_RPC_CONNECTION_H */
