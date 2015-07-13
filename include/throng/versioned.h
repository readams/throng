/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file versioned.h
 * @brief Interface definition file for versioned
 */
/*  * Copyright (c) 2015 Cisco Systems, Inc.
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
#ifndef THRONG_VERSIONED_H
#define THRONG_VERSIONED_H

#include "throng/vector_clock.h"

#include <cassert>
#include <memory>
#include <functional>

namespace throng {

/**
 * A value in the store combined with its version information.
 * Strings are treated as arbitrary binary data internally.
 */
template <typename V>
class versioned {
public:
    /**
     * Construct a new versioned (with move semantics enabled)
     *
     * @param value_ the value to set
     * @param version_ the version to set
     */
    versioned(const std::shared_ptr<const V> value_,
              const vector_clock version_)
        : value(std::move(value_)), version(std::move(version_)) { }

    /**
     * Check if there is an associated value
     * @return true if the value is set
     */
    explicit operator bool() const { return (bool)value; }

    /**
     * Get the value associated with this object.  Will assert if the
     * value is not set.  Before calling should check with operator
     * bool or use @ref value_or
     *
     * @return the value if it is set
     */
    const V& get() const { assert(value); return *value; }

    /**
     * Get a pointer to the value associated with this object.
     *
     * @return the pointer to the value, which could be a null ptr
     */
    const std::shared_ptr<const V>& get_ptr() const { return value; }

    /**
     * Get the value asscoiated with the object if it is set
     */
    const V& value_or(const V& v) const {
        return value ? *value : v;
    }

    /**
     * Get the version associated with this object
     */
    const vector_clock& get_version() const { return version; }

private:
    std::shared_ptr<const V> value;
    vector_clock version;
};

} /* namespace throng */

namespace std {

/**
 * Specialization for node ID hash
 */
template <>
struct hash<throng::node_id> {
    /**
     * Compute a hash for the node ID
     */
    size_t operator ()(throng::node_id value) const {
        size_t seed;
        std::hash<throng::node_id::value_type> hasher;
        for (auto n : value)
            seed ^= hasher(n) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        return seed;
    }
};

}

#endif /* THRONG_VERSIONED_H */
