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

//#include "throng/inconsistency_resolver.h"
#include "throng/ctx.h"
#include "throng/error.h"

#include <vector>
#include <functional>

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
template <typename K, typename V,
          class KS = serializer<K>,
          class VS = serializer<V>>
class store_client {
public:
    /**
     * An inconsistency resolver reduces a set of inconsistent data
     * values down to a single value.  A custom inconsistency resolver
     * can allow using commutative replicated data types such as sets,
     * counters, etc.
     */
    typedef std::function<std::vector<versioned<V>>
                          (std::vector<versioned<V>>&& items)>
    resolver_type;

    /**
     * A listener to get notifications for changes to keys in the
     * store.  It takes arguments of the changed key, and a bool that
     * indicates that the change notification occurred because of a
     * local write to the store.
     */
    typedef std::function<void(const K& key, bool local)> listener_type;

    /**
     * Get a store client to access a store that has already been
     * registered locally.
     *
     * @param context the throng context for the store
     * @param name the name of the store
     * @param resolver The custom inconsistency resolver to use
     * @return the new store client
     */
    static std::unique_ptr<store_client<K, V, KS, VS>>
        new_store_client(ctx& context, const std::string& name,
                         resolver_type resolver) {
        store<std::string,std::string>& delegate = context.get_raw_store(name);
        auto r = new store_client(context, delegate, resolver);
        return std::unique_ptr<store_client<K, V, KS, VS>>(r);
    }

    /**
     * Get a store client to access a store that has already been
     * registered locally.
     *
     * Uses the default inconsistency resolver which uses uses the
     * most recently-written concurrent data according to wall clock
     * timestamp.
     *
     * @param context the throng context for the store
     * @param name the name of the store
     * @return the new store client
     */
    static std::unique_ptr<store_client<K, V, KS, VS>>
        new_store_client(ctx& context, const std::string& name) {
        auto def_resolver =
        [](std::vector<versioned<V>>&& items) -> std::vector<versioned<V>> {
            if (items.size() <= 1) return items;
            auto& max = items.at(0);
            auto maxClock = max.get_version();
            auto maxTime = maxClock.get_timestamp();
            auto now = std::chrono::system_clock::now();
            for (auto& value : items) {
                auto& clock = value.get_version();
                if (clock.get_timestamp() > maxTime) {
                    max = value;
                    maxTime = clock.get_timestamp();
                }
                maxClock = maxClock.merge(clock, now);
            }
            std::vector<versioned<V>> result;
            result.emplace_back(max.get_ptr(), maxClock);
            return result;
        };
        return new_store_client(context, name, def_resolver);
    }

    /**
     * Get the value for the given key
     *
     * @param key the key to retrieve
     * @return a vector of values
     */
    versioned<V> get(const K& key) const {
        std::vector<versioned<std::string>> vs =
            delegate.get(key_ser.serialize(key));
        if (vs.size() == 0) return versioned<V>{ nullptr, {} };

        std::vector<versioned<V>> result;
        result.reserve(vs.size());
        for (auto& vv : vs) {
            if (vv) {
                result.emplace_back(value_ser.deserialize(vv.get_ptr()),
                                    vv.get_version());
            } else {
                result.emplace_back(nullptr, vv.get_version());
            }
        }

        result = resolver(std::move(result));
        if (result.size() == 1) return result.at(0);

        throw error::inconsistent_data(get_name(), result.size());
    }

    /**
     * Update the given versioned value in the store by writing
     *
     * @param key the key to store
     * @param old_value the old versioned value
     * @param new_value the new value
     * @throw error::obsolete_version if write is obsolete
     */
    void update(const K& key, const versioned<V>& old_value,
                const V& new_value) {
        vector_clock new_version(old_value.get_version()
                                 .incremented(context.get_local_node_id()));
        if (!delegate.put(key_ser.serialize(key),
                          versioned<std::string>(value_ser.serialize_ptr(new_value),
                                                 std::move(new_version))))
            throw error::obsolete_version();
    }

    /**
     * Delete the value associated with the key by writing a tombstone
     * value into the store.  Deletes any values prior to the given
     * version.
     *
     * @param key the key to delete
     * @param version the current version to delete (obtained with
     * get)
     * @throw error::obsolete_version if write is obsolete
     */
    void delete_key(const K& key, const vector_clock& version) {
        vector_clock new_version(version.incremented(context.get_local_node_id()));
        if (!delegate.put(key_ser.serialize(key),
                          versioned<std::string>(nullptr,
                                                 std::move(new_version))))
            throw error::obsolete_version();
    }

    /**
     * Get the name for this store.
     *
     * @return the name for the store
     */
    const std::string& get_name() const {
        return delegate.get_name();
    }

    /**
     * Register a listener to get notifications of changes to keys in
     * the store.
     */
    void register_listener(listener_type listener) {
        // XXX TODO
    }

private:
    /**
     * Construct a new store client
     *
     * @param context_ the throng context
     * @param delegate_ the store to which we should delegate
     * operations
     * @param resolver_ the inconsistency resolver to use
     */
    store_client(ctx& context_,
                 store<std::string, std::string>& delegate_,
                 resolver_type resolver_):
        context(context_), delegate(delegate_),
        resolver(std::move(resolver_)) { }

    ctx& context;
    store<std::string, std::string>& delegate;
    resolver_type resolver;
    KS key_ser;
    VS value_ser;
};

} /* namespace throng */

#endif /* THRONG_STORE_CLIENT_H */
