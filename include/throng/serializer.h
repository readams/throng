/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file serializer.h
 * @brief Interface definition file for serializer
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
#ifndef THRONG_SERIALIZER_H
#define THRONG_SERIALIZER_H

namespace throng {

/**
 * Serialize data from the internal binary string representation to
 * the "real" user-defined type
 */
template <typename V>
class serializer {
public:
    /**
     * Serialize the user-defined type into a string
     * @param val the value to serialize
     * @return a shared pointer to the new serialized value
     */
    std::shared_ptr<const std::string>
    serialize(const std::shared_ptr<const V>& val) const;

    /**
     * Serialize the user-defined type into a string
     * @param val the value to serialize
     * @return a shared pointer to the new serialized value
     */
    std::string serialize(const V& val) const;

    /**
     * Deserialize a string into the user-defined type
     * @param serialized the value to deserialize
     * @return a shared pointer to the new deserialized value
     */
    std::shared_ptr<const V>
    deserialize(const std::shared_ptr<const std::string>& serialized) const;
};

/**
 * Template specialization for @ref serializer.  Trivial data mapper
 * that simply maps strings to strings.
 */
template <>
class serializer<std::string> {
public:
    /**
     * Simply return the value
     * @param val the value to serialize
     * @return the input value
     * @see serialize::serialize
     */
    std::shared_ptr<const std::string>
    serialize(const std::shared_ptr<const std::string>& val) const {
        return val;
    }

    /**
     * Simply return the value
     * @param val the value to serialize
     * @return the input value
     * @see serialize::serialize
     */
    std::string serialize(const std::string& val) const {
        return val;
    }

    /**
     * Simply return the value
     * @param serialized the value to serialize
     * @return the input value
     * @see serialize::deserialize
     */
    std::shared_ptr<const std::string>
    deserialize(const std::shared_ptr<const std::string>& serialized) const {
        return serialized;
    }
};

} /* namespace throng */

#endif /* THRONG_SERIALIZER_H */
