/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for rpc_handler class.
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

#include "rpc_handler.h"
#include "rpc_connection.h"

namespace throng {
namespace internal {

rpc_handler::rpc_handler(ctx_internal& ctx_)
    : ctx(ctx_) {

}

#define HREQ(method)                                             \
    if (message.has_req() && message.req().has_##method()) {     \
        handle_req_##method(connection, message.xid(),           \
                            message.req().method());             \
    }
#define HREP(method)                                                    \
    if (message.has_rep()) {                                            \
        if (message.rep().status_code() != message::STATUS_OK) {        \
            handle_error_##method(connection,                           \
                                  message.xid(),                        \
                                  message.rep().status_code(),          \
                                  message.rep().status_message());      \
        } else if (message.rep().has_##method()) {                      \
            handle_rep_##method(connection, message.xid(),              \
                                message.rep().method());                \
        }                                                               \
    }

bool rpc_handler::handle_message(rpc_connection& connection,
                                 message::rpc_message& message) {
    if (!message.has_method()) {
        // XXX - TODO invalid message
        return false;
    }
    switch (message.method()) {
    case message::METHOD_HELLO:
        HREQ(hello);
        HREP(hello);
        break;
    //case message::METHOD_GET:
    //    HREQ(get);
    //    HREP(get);
    //    break;
    default:
        break;
    }
    return true;
}

bool rpc_handler::handle_connect(rpc_connection& connection) {
    message::rpc_message message;
    message.set_method(message::METHOD_HELLO);
    auto req = message.mutable_req()->mutable_hello();
    for (auto id : ctx.get_local_node_id())
        req->add_node_id(id);
    connection.send_message(message);
    return true;
}

void rpc_handler::handle_unsupported(rpc_connection& connection,
                                     uint64_t xid,
                                     message::method method) {
    connection.send_error_message(xid, method, message::STATUS_UNSUPPORTED,
                                  "Unsupported message");
}

void rpc_handler::handle_error(rpc_connection& connection,
                               message::method method,
                               message::status status_code,
                               const std::string& status_message) {
    // XXX TODO log error
    std::cerr << "Error for " << message::method_Name(method)
              << ": " << message::status_Name(status_code)
              << ": " << status_message << std::endl;
}

bool rpc_handler::handle_req_hello(rpc_connection& connection,
                                   uint64_t xid,
                                   const message::req_hello& message) {
    remote_node_id.clear();
    for (auto id : message.node_id())
        remote_node_id.push_back(id);

    std::cerr << "Got remote node ID: " << remote_node_id << std::endl;

    return true;
}

bool rpc_handler::handle_rep_hello(rpc_connection& connection,
                                   uint64_t xid,
                                   const message::rep_hello& message) {
    std::cerr << "Hello succeeded" << std::endl;
    return true;
}

void rpc_handler::handle_error_hello(rpc_connection& connection,
                                     uint64_t xid,
                                     message::status status_code,
                                     const std::string& status_message) {
    handle_error(connection, message::METHOD_HELLO,
                 status_code, status_message);
    connection.stop();
}

void rpc_handler::handle_req_get(rpc_connection& connection,
                                 uint64_t xid,
                                 const message::req_get& message) {
    handle_unsupported(connection, xid, message::METHOD_GET);
}

void rpc_handler::handle_rep_get(rpc_connection& connection,
                                 uint64_t xid,
                                 const message::rep_get& message) {

}

void rpc_handler::handle_error_get(rpc_connection& connection,
                                   uint64_t xid,
                                   message::status status_code,
                                   const std::string& status_message) {
    handle_error(connection, message::METHOD_GET,
                 status_code, status_message);
}

} /* namespace internal */
} /* namespace throng */
