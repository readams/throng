/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file processor.h
 * @brief Interface definition file for processor
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
#ifndef THRONG_PROCESSOR_H
#define THRONG_PROCESSOR_H

#include "ctx_internal.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/asio/steady_timer.hpp>

#include <mutex>
#include <unordered_map>
#include <chrono>

namespace throng {
namespace internal {

/**
 * Index and process the data in the store.  Keeps track of the data
 * in the store and handles periodic tasks, notifications, and
 * synchronization.
 */
class processor : public store<std::string, std::string> {
public:
    /**
     * Construct a new processor from the given delegate store.
     *
     * @param ctx_ the internal context
     * @param delegate_ the delegate store
     * @param config_ the configuration for the store
     */
    processor(ctx_internal& ctx_,
              std::unique_ptr<store<std::string, std::string>> delegate_,
              store_config config_)
        : ctx(ctx_), name(delegate_->get_name()), config(std::move(config_)),
          delegate(std::move(delegate_)) { }

    /**
     * Construct a new processor with in-memory storage using the given name
     *
     * @param ctx_ the internal context
     * @param name_ the name
     * @param config_ the configuration for the store
     */
    processor(ctx_internal& ctx_,
              std::string name_, store_config config_)
        : ctx(ctx_), name(std::move(name_)), config(std::move(config_)) { }
    virtual ~processor() {};

    // *********
    // processor
    // *********

    /**
     * Start the processor
     */
    virtual void start();

    /**
     * Stop the processor
     */
    virtual void stop();

    /**
     * Add a listener for this store
     *
     * @param listener the listener to add
     */
    virtual void add_listener(ctx::raw_listener_t listener) {
        listeners.push_back(std::move(listener));
    }

    // ********************
    // store<string,string>
    // ********************

    virtual std::vector<versioned<std::string>>
    get(const std::string& key) override;
    virtual bool put(const std::string& key,
                     const versioned_t& value) override;
    virtual const std::string& get_name() const override;
    virtual void visit(store_visitor visitor) override;

private:
    /**
     * The internal context object
     */
    ctx_internal& ctx;

    /**
     * The name of the store associated with this processor
     */
    std::string name;

    /**
     * The configuration for the store
     */
    store_config config;

    /**
     * If present, values from the store are written also to the
     * delegate object, we'll read from this object on startup to
     * restore persisted state.
     */
    std::unique_ptr<store<std::string, std::string>> delegate;

    /**
     * Listeners that will be notified when data in the store is
     * updated
     */
    std::vector<ctx::raw_listener_t> listeners;

    /**
     * True if the store is still running
     */
    volatile bool running = false;

    typedef std::chrono::steady_clock::time_point time_point;

    struct item_details {
        uint32_t key_hash[5];
        std::vector<versioned_t> values;
        time_point last_refresh;
        time_point last_resolve;
    };

    struct item {
        item(std::string key_, time_point next_time_)
            : details(new item_details()),
              key(std::move(key_)), next_time(next_time_) {
            boost::uuids::detail::sha1 sha1;
            sha1.process_bytes(key.c_str(), key.size());
            sha1.get_digest(details->key_hash);
        }

        item() = delete;
        item(const item&) = delete;
        item& operator=(const item&) = delete;
        item(item&&) = default;
        item& operator=(item&&) = default;

        std::unique_ptr<item_details> details;

        std::string key;
        time_point next_time;
    };

    // tag for next_time index
    struct next_time_tag{};
    // tag for key index
    struct key_tag{};

    struct item_indexes :
        boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
            boost::multi_index::tag<key_tag>,
            boost::multi_index::member<item,
                                       std::string,
                                       &item::key> > ,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<next_time_tag>,
            boost::multi_index::member<item,
                                       time_point,
                                       &item::next_time> >
        > {};

    typedef boost::multi_index::multi_index_container<
        item, item_indexes
        > item_map_t;

    /**
     * Mutex for synchronization of store data
     */
    std::mutex item_mutex;

    /**
     * The data in the store along with the necessary metadata
     */
    item_map_t item_map;

    std::unique_ptr<boost::asio::steady_timer> proc_timer;

    typedef item_map_t::index<next_time_tag>::type item_map_by_time;

    void on_proc_timer(const boost::system::error_code& ec);
    void process(item_map_by_time::iterator& it);
    void notify(const std::string& key, bool local);
    bool doput(item_details& rs,
               const versioned<std::string>& value);
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_PROCESSOR_H */
