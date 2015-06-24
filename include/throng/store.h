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
     * Get the set of values associated with a given key, or an empty
     * vector if there are no such values.
     *
     * @param key the key to retrieve
     * @return a vector of values
     */
    virtual std::vector<versioned<V>> get(const K& key) const = 0;

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

    /**
     * Close the store
     */
    virtual void close() = 0;
};

/**
 * Store interface that maps from the internal binary string
 * representation to the external represenation
 */
template <typename K, typename V,
          class KeySerializer = serializer<K>,
          class ValueSerializer = serializer<V>>
class serializing_store : public store<K,V> {
public:
    /**
     * Construct a new serializing store using the specified delegate
     *
     * @param delegate_ the underlying delegate store
     */
    serializing_store(store<std::string, std::string>& delegate_)
        : delegate(delegate_) { }

    virtual std::vector<versioned<V>> get(const K& key) const override {
        std::vector<versioned<std::string>> vs =
            delegate.get(key_ser.serialize(key));
        std::vector<versioned<V>> result;
        result.reserve(vs.size());
        for (auto vv : vs) {
            if (vv) {
                result.emplace_back(value_ser.deserialize(vv.get_ptr()),
                                    vv.get_version());
            } else {
                result.emplace_back(nullptr, vv.get_version());
            }
        }
        return result;
    }

    virtual void put(const K& key, const versioned<V>& value) override {
        delegate.put(key_ser.serialize(key),
                     versioned<std::string>(value
                                            ? value_ser.serialize(value.get_ptr())
                                            : nullptr,
                                            value.get_version()));
    }

    virtual bool deleteKey(const K& key, const vector_clock& version) override {
        return delegate.deleteKey(key_ser.serialize(key), version);
    };

    virtual const std::string& getName() const override {
        return delegate.getName();
    };

    virtual void close() override {
        return delegate.close();
    };

private:
    store<std::string, std::string>& delegate;
    KeySerializer key_ser;
    ValueSerializer value_ser;
};

} /* namespace throng */

#endif /* THRONG_STORE_H */
