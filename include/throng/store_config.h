/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file ctx.h
 * @brief Interface definition file for throng ctx
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
#ifndef THRONG_STORE_CONFIG_H
#define THRONG_STORE_CONFIG_H

#include <chrono>

namespace throng {

/**
 * Configuration for a store
 */
struct store_config {
    /**
     * Set to true to enable persistence for this store.  False means
     * data is stored only in memory.
     */
    bool persistent = false;

    /**
     * The number of replicas for objects written to this store
     */
    uint8_t rep_factor = 3;

    /**
     * The timeout for an object that has not been updated for some
     * period of time.  This is useful for objects that might be
     * written to a store but correspond to stale state.  Data written
     * by the local node is automatically refreshed.
     */
    std::chrono::seconds object_timeout = std::chrono::seconds::zero();

    /**
     * The timeout before removing a tombstone corresponding to a
     * deleted object.  This timeout represents the maximum amount of
     * time a network partition can be tolerated before there is a
     * possibility that deleted objects could be "resurrected" by a
     * healing of the partition.
     */
    std::chrono::seconds tombstone_timeout = std::chrono::hours(24);

};

} /* namespace throng */

#endif /* THRONG_STORE_CONFIG_H */
