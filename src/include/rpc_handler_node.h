/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file rpc_handler_node.h
 * @brief Interface definition file for rpc_handler_node
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
#ifndef THRONG_RPC_HANDLER_NODE_H
#define THRONG_RPC_HANDLER_NODE_H

#include "rpc_handler.h"

namespace throng {
namespace internal {

class rpc_connection;

/**
 * Handles RPC messages for a node instance
 */
class rpc_handler_node : public rpc_handler {
public:
    /**
     * Create a new RPC handler
     */
    rpc_handler_node(ctx_internal& ctx);
    virtual ~rpc_handler_node() {};

    // ****************
    // Message handlers
    // ****************

    virtual bool handle_req_hello(rpc_connection& connection,
                                  uint64_t xid,
                                  const message::req_hello& message) override;
    virtual bool handle_rep_hello(rpc_connection& connection,
                                  uint64_t xid,
                                  const message::rep_hello& message) override;

    virtual void handle_req_get(rpc_connection& connection,
                                uint64_t xid,
                                const message::req_get& message) override;

private:
    internal::logger lgr = LOGGER("rpc_handler_node");
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_RPC_HANDLER_NODE_H */
