/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file serializer_protobuf.h
 * @brief Interface definition file for serializer for protobuf
 * messages
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
#ifndef THRONG_SERIALIZER_PROTOBUF_H
#define THRONG_SERIALIZER_PROTOBUF_H

#include "serializer.h"

#include <google/protobuf/message_lite.h>

namespace throng {

/**
 * Template specialization for a google protobuf message.
 */
template <class _MessageLite>
class serializer<_MessageLite,
                 typename std::enable_if<std::is_base_of<google::protobuf::MessageLite, _MessageLite>::value>::type> {
public:
    /**
     * Serialize the user-defined type into a string
     * @param val the value to serialize
     * @return a shared pointer to the new serialized value
     */
    std::shared_ptr<const std::string>
    serialize_ptr(const _MessageLite& val) const {
        auto result = std::make_shared<std::string>();
        val.SerializeToString(result.get());
        return result;
    }

    /**
     * Serialize the user-defined type into a string
     * @param val the value to serialize
     * @return the new serialized value
     */
    std::string serialize(const _MessageLite& val) const {
        std::string result;
        val.SerializeToString(&result);
        return result;
    }

    /**
     * Deserialize a string into the user-defined type
     * @param serialized the value to deserialize
     * @return a shared pointer to the new deserialized value
     */
    std::shared_ptr<const _MessageLite>
    deserialize(const std::shared_ptr<const std::string>& serialized) const {
        auto result = std::make_shared<_MessageLite>();
        result->ParseFromString(*serialized);
        return result;
    }

    /**
     * Deserialize a string into the user-defined type
     * @param serialized the value to deserialize
     * @return the new deserialized value
     */
    _MessageLite deserialize(const std::string& serialized) const {
        _MessageLite result;
        result.ParseFromString(serialized);
        return result;
    }
};

} /* namespace throng */

#endif /* THRONG_SERIALIZER_PROTOBUF_H */
