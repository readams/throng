/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file cluster_config.h
 * @brief Interface definition file for cluster_config
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
#ifndef THRONG_CLUSTER_CONFIG_H
#define THRONG_CLUSTER_CONFIG_H

#include "throng_messages.pb.h"

#include "throng/vector_clock.h"
#include "throng/store_client.h"
#include "throng/serializer_protobuf.h"

#include <boost/optional.hpp>

#include <unordered_map>
#include <memory>

THRONG_PROTOBUF_SERIALIZER(throng::message::node)
THRONG_PROTOBUF_SERIALIZER(throng::message::node_id)
THRONG_PROTOBUF_SERIALIZER(throng::message::neighborhood)

namespace throng {
namespace internal {

/**
 * A store client for the system node store
 */
typedef store_client<message::node_id, message::node> node_client_t;

/**
 * A store client for the system neighborhood store
 */
typedef store_client<message::node_id, message::neighborhood> neigh_client_t;

/**
 * The name of the system node store
 */
extern const std::string NODE_STORE;

/**
 * The name of the system neighborhood store
 */
extern const std::string NEIGH_STORE;

/**
 * Provides the current configuration for the cluster.  This object
 * should be treated as immutable after initial construction
 */
class cluster_config {
public:
    /**
     * Create a new cluster config service instance
     */
    cluster_config();

    /**
     * A shared pointer to a neighborhood
     */
    typedef std::shared_ptr<message::neighborhood> neigh_p;

    /**
     * Add the neighborhood
     *
     * @param neigh the neighborhood to add
     */
    void add_neighborhood(neigh_p neigh);

    /**
     * Get a neighborhood by its ID.
     *
     * @param id the ID of the neighborhood
     * @param neigh_p the corresponding neighborhood if it exists, or
     * boost::none otherwise.
     */
    boost::optional<neigh_p> get_neighborhood(node_id& id) const;

    /**
     * Get all neighborhoods
     *
     * @return the neighborhood map
     */
    const std::unordered_map<node_id, neigh_p>& get_neighborhoods() const;

private:
    /**
     * Map from neighborhood prefix to the current cached neighborhood
     * object
     */
    std::unordered_map<node_id, neigh_p> neighs;

    /**
     * The buckets assigned to the local node.  The map is:
     * scope -> id -> replication number
     */
    std::unordered_map<uint8_t, std::unordered_map<uint16_t, uint8_t>> buckets;
};

/**
 * A shared pointer to a cluster configuration
 */
typedef std::shared_ptr<const cluster_config> cluster_config_p;

/**
 * A provider of cluster config will return the most current cluster
 * configuration
 */
typedef std::function<cluster_config_p()> config_provider;

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_CLUSTER_CONFIG_H */
