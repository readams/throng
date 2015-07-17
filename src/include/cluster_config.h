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

#include "ctx_internal.h"
#include "throng_messages.pb.h"

#include <unordered_map>

namespace throng {
namespace internal {

/**
 * Provides the current configuration for the cluster
 */
class cluster_config {
public:
    /**
     * Create a new cluster config service instance
     *
     * @param ctx the internal context
     */
    cluster_config();

private:
    /**
     * Map from neighborhood prefix to the current cached neighborhood
     * object
     */
    std::unordered_map<node_id, message::neighborhood> neighs;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_CLUSTER_CONFIG_H */
