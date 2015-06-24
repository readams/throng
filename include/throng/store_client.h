/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file store_client.h
 * @brief Interface definition file for store_client
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
#ifndef THRONG_STORE_CLIENT_H
#define THRONG_STORE_CLIENT_H

#include "throng/versioned.h"

#include <vector>

namespace throng {

/**
 * A store_client is the primary means of accessing data in a throng
 * database.  The store client handles:
 *  * storing and retrieving data from the store
 *  * mapping data to/from its serialized representation
 *  * resolving conflicts in the data
 *  * registering listeners for changes in the data
 *  * registering keys for resolution
 */
template <typename K, typename V>
class store_client {
public:
    /**
     * Get the value for the given key
     *
     * @param key the key to retrieve
     * @return a vector of values
     */
    virtual versioned<V> get(const K& key) const = 0;

    /**
     * Put the given value into the store
     *
     * @param key the key to store
     * @param value the value to store
     */
    virtual void put(const K& key, const versioned<V>& value) = 0;

    /**
     * Delete the value associated with the key by writing a tombstone
     * value into the store.  Deletes any values prior to the given
     * version.
     *
     * @param key the key to delete
     * @param version the current version to delete (obtained with
     * get)
     * @return true if anything was deleted
     */
    virtual bool deleteKey(const K& key, const vector_clock& version) = 0;

    /**
     * Get the name for this store.
     *
     * @return the name for the store
     */
    virtual const std::string& getName() const = 0;
};

} /* namespace throng */

#endif /* THRONG_STORE_CLIENT_H */
