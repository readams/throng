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

#include "throng/vector_clock.h"
#include "rpc_handler_node.h"
#include "rpc_connection.h"
#include "logger.h"

namespace throng {
namespace internal {

LOGGER("rpc");

rpc_handler_node::rpc_handler_node(ctx_internal& ctx_)
    : rpc_handler(ctx_) {

}

bool rpc_handler_node::handle_req_hello(rpc_connection& connection,
                                        uint64_t xid,
                                        const message::req_hello& message) {
    if (!rpc_handler::handle_req_hello(connection, xid, message))
        return false;

    message::rpc_message reply;
    reply.set_method(message::METHOD_HELLO);
    reply.mutable_rep()->mutable_hello();
    connection.send_message(reply);
    LOG(INFO) << HTAG << "Handshake succeeded";

    return handle_ready(connection);
}

bool rpc_handler_node::handle_rep_hello(rpc_connection& connection,
                                        uint64_t xid,
                                        const message::rep_hello& message) {
    return rpc_handler::handle_rep_hello(connection, xid, message);
}

void rpc_handler_node::handle_req_get(rpc_connection& connection,
                                      uint64_t xid,
                                      const message::req_get& message) {
    handle_unsupported(connection, xid, message::METHOD_GET);
}

} /* namespace internal */
} /* namespace throng */
