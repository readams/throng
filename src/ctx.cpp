/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for ctx class.
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

#include "throng/ctx.h"
#include "throng_messages.pb.h"
#include "store_registry.h"

//#include <leveldb/db.h>

namespace throng {

using std::string;
using std::unique_ptr;

class ctx::ctx_impl {
public:
    internal::store_registry registry;
    //unique_ptr<leveldb::DB> systemDb;
};

ctx::ctx(string dbPath)
    : pimpl(new ctx_impl { { dbPath } }) {

}
ctx::~ctx() {

}

void ctx::register_store(const std::string& name) {
    register_store(name, store_config {});
}

void ctx::register_store(const std::string& name,
                         const store_config& config) {
    pimpl->registry.register_store(name, config);
}

void ctx::start() {
//    leveldb::Options options;
//    options.create_if_missing = true;
//    leveldb::DB* dbPtr = nullptr;
//    path systemDbPath = pimpl->dbPath;
//    systemDbPath /= "__system__";
//    leveldb::Status status = leveldb::DB::Open(options,
//                                               systemDbPath.string(),
//                                               &dbPtr);
//    if (!status.ok()) {
//        throw std::runtime_error(string("Could not create/open database: ") +
//                                 status.ToString());
//    } else {
//        pimpl->systemDb.reset(dbPtr);
//        dbPtr = nullptr;
//    }
}

void ctx::stop() {

}

store<string,string>& ctx::get_raw_store(const string& name) {
    return pimpl->registry.get(name);
}

node_id ctx::get_local_node_id() const {
    // XXX TODO
    return {1};
}

} /* namespace throng */
