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

#include "in_memory_storage_engine.h"

namespace throng {
namespace internal {

using std::vector;
using std::string;

vector<versioned<string>>
in_memory_storage_engine::get(const string& key) const {
    return {};
}

void in_memory_storage_engine::put(const string& key,
                                   const versioned<string>& value) {

}

bool in_memory_storage_engine::deleteKey(const string& key,
                                         const vector_clock& version) {
    return false;
}

const string& in_memory_storage_engine::getName() const {
    return name;
}

void in_memory_storage_engine::close() {

}

} /* namespace internal */
} /* namespace throng */
