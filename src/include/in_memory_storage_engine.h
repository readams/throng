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

#include "throng/store.h"

#include <mutex>
#include <unordered_map>

namespace throng {
namespace internal {

/**
 * The in-memory storage engine implements a storage engine that
 * stores its data in-memory non-persistently.
 */
class in_memory_storage_engine : public store<std::string, std::string> {
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
    virtual std::vector<versioned_t>
    get(const std::string& key) override;

    virtual bool put(const std::string& key,
                     const versioned_t& value) override;

    /**
     * Get the name for this store.
     *
     * @return the name for the store
     */
    virtual const std::string& get_name() const override;

private:
    std::string name;
    std::mutex lock;

    struct record {
        std::vector<versioned_t> values;
    };

    std::unordered_map<std::string, record> records;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_IN_MEMORY_STORAGE_ENGINE_H */
