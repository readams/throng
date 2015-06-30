/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file storage_engine.h
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
#ifndef THRONG_STORAGE_ENGINE_H
#define THRONG_STORAGE_ENGINE_H

#include <throng/store.h>

#include <string>

namespace throng {
namespace internal {

/**
 * A storage engine represents a key/value data store that is local to
 * the current node.  It supports additional operations required for
 * synchronizing and managing the state of the data in the store.
 */
class storage_engine : public store<std::string, std::string> {
public:
    /**
     * The type of versioned value used by a storage engine
     */
    typedef versioned<std::string> versioned_type;

    virtual ~storage_engine() {};

};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_STORAGE_ENGINE_H */
