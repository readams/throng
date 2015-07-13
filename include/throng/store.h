/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file store.h
 * @brief Interface definition file for store
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
#ifndef THRONG_STORE_H
#define THRONG_STORE_H

#include "throng/versioned.h"
#include "throng/serializer.h"

#include <vector>

namespace throng {

/**
 * A store is an interface that defines methods for accessing data
 * from the throng distributed database.  Note that this allows access
 * only to data which has been resolved and synchronized locally.
 *
 * Users of the database will generally want to access data through a
 * store_client since this will provide additional services such as
 * conflict resolution.
 */
template <typename K, typename V>
class store {
public:
    virtual ~store() {};

    /**
     * The type of versioned value used by the store
     */
    typedef versioned<V> versioned_type;

    /**
     * Get the set of values associated with a given key, or an empty
     * vector if there are no such values.
     *
     * @param key the key to retrieve
     * @return a vector of values
     */
    virtual std::vector<versioned_type> get(const K& key) = 0;

    /**
     * Put the given value into the store
     *
     * @param key the key to store
     * @param value the value to store
     * @return true if the value was successfully written, or false if
     * the new value is obsolete
     */
    virtual bool put(const K& key, const versioned_type& value) = 0;

    /**
     * Get the name for this store.
     *
     * @return the name for the store
     */
    virtual const std::string& get_name() const = 0;
};

} /* namespace throng */

#endif /* THRONG_STORE_H */
