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
#ifndef THRONG_CTX_H
#define THRONG_CTX_H

#include "throng/store_client.h"
#include "throng/store.h"
#include "throng/serializer.h"
#include "throng/store_config.h"

#include <string>
#include <memory>

namespace throng {

/**
 * Provides configuration and state management for the throng cluster
 */
class ctx {
public:
    /**
     * Create a new throng context instance with storage in the
     * specified db path.
     *
     * @param db_path filesystem path for permanent storage.
     */
    ctx(const std::string db_path);
    ~ctx();

    /**
     * Start the throng instance and connect to the cluster
     */
    void start();

    /**
     * Stop the throng context instance
     */
    void stop();

    /**
     * Register a store with the specified name and default
     * configuration.  The default config will be a
     * default-initialized store_config.
     *
     * @param name the name for the store
     * @see register_store(const std::string&, const store_config&)
     */
    void register_store(const std::string& name);

    /**
     * Register a store with the specified name and configuration.
     * All stores must be registered before calling start()
     *
     * @param name the name for the store
     * @param config the configuration to use for the store.
     */
    void register_store(const std::string& name,
                        const store_config& config);

    /**
     * Get a store client to access a store that has already been
     * registered locally.
     */
    template <typename K, typename V,
              class KeySerializer = serializer<K>,
              class ValueSerializer = serializer<V>>
        std::unique_ptr<store_client<K, V>>
        get_store_client(const std::string& name) {
        store<std::string,std::string>* delegate = get_raw_store(name);
        serializing_store<K,V,KeySerializer,ValueSerializer> ss(*delegate);
        return nullptr;
    }

private:
    /**
     * Get a raw reference to the underlying store
     */
    store<std::string,std::string>* get_raw_store(const std::string& name);

    class ctx_impl;
    std::unique_ptr<ctx_impl> pimpl;
    friend class ctx_impl;
};

} /* namespace throng */

#endif /* THRONG_CTX_H */
