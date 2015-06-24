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
#ifndef THRONG_PROTOBUF_SERIALIZER
/**
 * Create a template specialization for a protocol buffers message to
 * allow storing it in throng store easily
 */
#define THRONG_PROTOBUF_SERIALIZER(_MessageLite)                        \
    namespace throng {                                                  \
    template <> class serializer<_MessageLite> {                        \
        public:                                                         \
        std::shared_ptr<const std::string>                              \
        serialize(const std::shared_ptr<const _MessageLite>& val) const { \
            auto result = std::make_shared<std::string>();              \
            val->SerializeToString(result.get());                       \
            return result;                                              \
        }                                                               \
                                                                        \
        std::string serialize(const _MessageLite& val) const {          \
            std::string result;                                         \
            val.SerializeToString(&result);                             \
            return result;                                              \
        }                                                               \
                                                                        \
        std::shared_ptr<const _MessageLite>                             \
        deserialize(const std::shared_ptr<const std::string>& serialized) const { \
            auto result = std::make_shared<_MessageLite>();             \
            result->ParseFromString(*serialized);                       \
            return result;                                              \
        }                                                               \
    };}

#endif /* THRONG_PROTOBUF_SERIALIZER */
