/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for store_registry class.
 *
 * Copyright (c) 2015 Cisco Systems, Inc.
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

#include "store_registry.h"
#include "throng/error.h"

namespace throng {
namespace internal {

using std::unordered_map;
using std::string;
using std::unique_ptr;
using boost::filesystem::path;
using boost::filesystem::canonical;

store_registry::store_registry(const string db_path_) {
    boost::filesystem::create_directory(db_path_);
    db_path = canonical(db_path);
}

void store_registry::register_store(const std::string& name,
                                    const store_config& config) {
    typedef std::unique_ptr<storage_engine> storage_engine_p;

    storage_engine_p underlying;
    if (config.persistent) {
        // XXX - TODO replace with persistent storage engine
        underlying = storage_engine_p{ new in_memory_storage_engine(name) };
    } else {
        underlying = storage_engine_p{ new in_memory_storage_engine(name) };
    }
    // XXX - TODO wrap with synchronizing storage engine
    stores.emplace(name, std::move(underlying));
}

storage_engine& store_registry::get(const std::string& name) {
    auto it = stores.find(name);
    if (it == stores.end())
        throw error::unknown_store(name);
    return *it->second.get();
}

} /* namespace internal */
} /* namespace throng */
