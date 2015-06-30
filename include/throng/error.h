/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file error.h
 * @brief Interface definition file for throng errors and exceptions
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
#ifndef THRONG_ERROR_H
#define THRONG_ERROR_H

#include <stdexcept>
#include <typeinfo>

namespace throng {
namespace error {

/**
 * Base exception that all other throng exceptions extend.
 */
class exception: public std::runtime_error
{
public:
    /**
     * Create a new exception with the provided message
     *
     * @param message The error message
     */
    exception(const std::string& message) :
        runtime_error(message) { }
};

/**
 * Thrown when attempting to perform an operation on an unregistered
 * store.
 */
class unknown_store: public exception
{
public:
    /**
     * Create a new exception with the provided message
     *
     * @param store The name of the unknown store
     */
   unknown_store(const std::string& store) :
       exception("Unknown store: " + store) { }
};

/**
 * Thrown when attempting to perform an operation on an unregistered
 * store.
 */
class serialization: public exception
{
public:
    /**
     * Create a new exception with the provided message
     *
     * @param message The error message
     */
   serialization(const std::string& message) :
        exception(message) { }
};

/**
 * Thrown when a write to a local data store is older than all
 * existing values for the same data
 */
class obsolete_version: public exception
{
public:
    /**
     * Create a new exception with the provided message
     */
   obsolete_version() :
       exception("Obsolete write") { }
};

/**
 * Thrown when an inconsistency resolver fails to reduce to a single
 * element.
 */
class inconsistent_data: public exception
{
public:
    /**
     * Create a new exception with the provided message
     *
     * @param store_name The store name for the failed resolver
     * @param elements the number of elements remaining
     */
    inconsistent_data(const std::string& store_name,
                      size_t elements) :
        exception("Inconsistent data for " + store_name +
                  ": " + std::to_string(elements) + " remaining") { }
};

} /* namespace error */
} /* namespace throng */

#endif /* THRONG_ERROR_H */
