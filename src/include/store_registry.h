/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file store_registry.h
 * @brief Interface definition file for store registry
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
#ifndef THRONG_STORE_REGISTRY_H
#define THRONG_STORE_REGISTRY_H

#include "throng/store_config.h"
#include "in_memory_storage_engine.h"

#include <boost/filesystem.hpp>
#include <unordered_map>
#include <memory>

namespace throng {
namespace internal {

/**
 * Provides configuration and state management for the throng cluster
 */
class store_registry {
public:
    /**
     * Create a new throng context instance with storage in the
     * specified db path.
     *
     * @param db_path filesystem path for permanent storage.
     */
    store_registry(const std::string db_path);

    /**
     * Register a store with the registry.
     *
     * @param name the name of the store
     * @param scope the scope for the store.
     * @param persistent true if the data in the store should be
     * stored on disk
     */
    void register_store(const std::string& name,
                        const store_config& config);

    /**
     * Get the storage engine for the store with the specified name
     *
     * @param name the name of the store to get
     * @return a pointer to the storage engine.  This pointer will
     * live as long as the store registry.
     * @throws error::unknown_store if there is no such store
     */
    storage_engine* get(const std::string& name);

private:
    /**
     * Filesystem location for persistent data stores
     */
    boost::filesystem::path db_path;

    /**
     * Storage engines for local copies of the stores.
     */
    std::unordered_map<std::string, std::unique_ptr<storage_engine>> stores;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_STORE_REGISTRY_H */
