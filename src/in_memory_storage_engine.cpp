/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for in_memory_storage_engine class.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "in_memory_storage_engine.h"

#include <utility>

namespace throng {
namespace internal {

using std::vector;
using std::string;

vector<versioned<string>>
in_memory_storage_engine::get(const string& key) {
    std::lock_guard<std::mutex> guard(lock);
    vector<versioned<string>> result;
    auto it = records.find(key);
    if (it == records.end()) return result;
    for (auto& v : it->second.values) {
        result.push_back(v);
    }
    return result;
}

bool in_memory_storage_engine::put(const string& key,
                                   const versioned<string>& value) {
    std::lock_guard<std::mutex> guard(lock);

    record& rs = records[key];
    std::vector<versioned_t> new_values;

    for (auto vit = rs.values.begin(); vit != rs.values.end(); vit++) {
        vector_clock::occurred o =
            value.get_version().compare(vit->get_version());

        switch (o) {
        case vector_clock::occurred::BEFORE:
        case vector_clock::occurred::EQUAL:
            return false;
        case vector_clock::occurred::AFTER:
            break;
        default:
            new_values.push_back(*vit);
            break;
        }
    }
    new_values.push_back({ value });
    rs.values.swap(new_values);
    return true;
}

const string& in_memory_storage_engine::get_name() const {
    return name;
}

} /* namespace internal */
} /* namespace throng */
