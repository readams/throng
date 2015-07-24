/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for cluster_config.
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

#include "cluster_config.h"
#include "logger.h"

#include <mutex>
#include <iostream>
#include <atomic>

namespace throng {
namespace internal {

using std::unordered_map;
using boost::optional;

LOGGER("core");

const std::string NODE_STORE = "__sys_node_store";
const std::string NEIGH_STORE = "__sys_neigh_store";

cluster_config::cluster_config()
     {}

void cluster_config::add_neighborhood(neigh_p neigh) {
    node_id prefix;
    if (!neigh->has_prefix()) return;

    for (auto i : neigh->prefix().id())
        prefix.push_back(i);

    neighs.insert({std::move(prefix), std::move(neigh)});
}

/**
 * Get a neighborhood by its ID.
 *
 * @param id the ID of the neighborhood
 * @param neigh_p the corresponding neighborhood if it exists, or
 * boost::none otherwise.
 */
optional<cluster_config::neigh_p>
cluster_config::get_neighborhood(node_id& id) const {
    auto it = neighs.find(id);
    if (it != neighs.end()) return it->second;
    return boost::none;
}

/**
 * Get all neighborhoods
 *
 * @return the neighborhood map
 */
const decltype(cluster_config::neighs)&
cluster_config::get_neighborhoods() const {
    return neighs;
}

} /* namespace internal */
} /* namespace throng */
