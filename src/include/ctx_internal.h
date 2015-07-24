/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file ctx_internal.h
 * @brief Interface definition file for throng ctx_internal
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
#ifndef THRONG_CTX_INTERNAL_H
#define THRONG_CTX_INTERNAL_H

#include "cluster_config.h"

#include "throng/ctx.h"

#include <boost/asio/io_service.hpp>

namespace throng {
namespace internal {

class rpc_service;

/**
 * Provides access to library-wide services for internal objects
 */
class ctx_internal : public ctx {
public:
    virtual ~ctx_internal() {}

    /**
     * Get the IO service
     *
     * @return the IO service object
     */
    virtual boost::asio::io_service& get_io_service() = 0;

    /**
     * Get the RPC service
     *
     * @return the RPC service object
     */
    virtual rpc_service& get_rpc_service() = 0;

    /**
     * A seed for bootstrapping the cluster
     */
    typedef std::pair<std::string, uint16_t> seed_type;

    /**
     * Get the seed information for the local node
     *
     * @return the hostname and port number for the local node
     */
    virtual seed_type& get_local_seed() = 0;

    /**
     * Get the current cluster configuration
     *
     * @return a shared pointer to the current cluster configuration
     */
    virtual cluster_config_p get_cluster_config() = 0;

    /**
     * Set a static node configuration for the cluster
     *
     * @param config the static configuration to set
     */
    virtual void set_static_config(cluster_config_p config) = 0;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_CTX_INTERNAL_H */
