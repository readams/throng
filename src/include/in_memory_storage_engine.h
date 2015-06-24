/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file in_memory_storage_engine.h
 * @brief Interface definition file for storage_engine
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
#ifndef THRONG_IN_MEMORY_STORAGE_ENGINE_H
#define THRONG_IN_MEMORY_STORAGE_ENGINE_H

#include "storage_engine.h"

namespace throng {
namespace internal {

/**
 * The in-memory storage engine implements a storage engine that
 * stores its data in-memory non-persistently.
 */
class in_memory_storage_engine : public storage_engine {
public:
    /**
     * Construct a new in-memory storage engine with the given name.
     *
     * @param name_ the name for the storage engine
     */
    in_memory_storage_engine(const std::string name_)
        : name(std::move(name_)) { }
    virtual ~in_memory_storage_engine() {};

    // ********************
    // store<string,string>
    // ********************

    /**
     * Get the set of values associated with a given key, or an empty
     * vector if there are no such values.
     *
     * @param key the key to retrieve
     * @return a vector of values
     */
    virtual std::vector<versioned<std::string>>
    get(const std::string& key) const override;

    /**
     * Put the given value into the store
     *
     * @param key the key to store
     * @param value the value to store
     */
    virtual void put(const std::string& key,
                     const versioned<std::string>& value) override;

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
    virtual bool deleteKey(const std::string& key,
                           const vector_clock& version) override;

    /**
     * Get the name for this store.
     *
     * @return the name for the store
     */
    virtual const std::string& getName() const override;

    /**
     * Close the store
     */
    virtual void close() override;

    // **************
    // storage_engine
    // **************
private:
    std::string name;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_IN_MEMORY_STORAGE_ENGINE_H */
