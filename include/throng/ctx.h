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
     * @return a newly-allocated pointer
     */
    static std::unique_ptr<ctx> new_ctx(const std::string db_path);

    virtual ~ctx() {}

    /**
     * Set the configuration for the local node.
     *
     * @param node_id the node ID for the local node
     * @param hostname the hostname or IP address for contacting this
     * node from the other nodes
     * @param port the port number to use for contacting this node
     * from other nodes.
     * @param master_eligible true if this node can act as a
     * neighborhood master
     */
    virtual void configure_local(node_id node_id,
                                 std::string hostname, uint16_t port,
                                 bool master_eligible = true) = 0;

    /**
     * Add a seed for connecting to the cluster
     *
     * @param hostname the hostname or IP address
     * @param port the port number
     */
    virtual void add_seed(std::string hostname, uint16_t port) = 0;

    /**
     * Register a store with the specified name and default
     * configuration.  The default config will be a
     * default-initialized store_config.
     *
     * @param name the name for the store
     * @see register_store(const std::string&, const store_config&)
     */
    virtual void register_store(const std::string& name) = 0;

    /**
     * Register a store with the specified name and configuration.
     * All stores must be registered before calling start()
     *
     * @param name the name for the store
     * @param config the configuration to use for the store.
     */
    virtual void register_store(const std::string& name,
                                const store_config& config) = 0;

    /**
     * Start the throng instance and connect to the cluster
     *
     * @param worker_pool_size the number of IO service worker threads
     * to create
     */
    virtual void start(size_t worker_pool_size = 3) = 0;

    /**
     * Stop the throng context instance
     */
    virtual void stop() = 0;

    /**
     * Get the node ID for the local node
     */
    virtual node_id get_local_node_id() = 0;

    /**
     * A listener to get notifications for changes to keys in the
     * store.  It takes arguments of the changed key, and a bool that
     * indicates that the change notification occurred because of a
     * local write to the store.
     */
    typedef std::function<void(const std::string& key, bool local)>
    raw_listener_type;

    /**
     * Register a listener to get raw notifications for the specified
     * store.  Note that this is almost never what you want.  Instead,
     * create a store_client and register your listener to get
     * typesafe notifications.
     *
     * @param store_name the store on which to register the listener
     * @param listener the listener to register.
     */
    virtual void add_raw_listener(const std::string& store_name,
                                  raw_listener_type listener) = 0;

    /**
     * Get a raw reference to the underlying store.  Note that this is
     * almost never what you want.  Instead, create a store_client to
     * access the store.
     *
     * @param name the name of the store
     * @return a reference to the raw store
     */
    virtual store<std::string,std::string>&
    get_raw_store(const std::string& name) = 0;
};

} /* namespace throng */

#endif /* THRONG_CTX_H */
