/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file test_util.h
 * @brief Interface definition file for test_util
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
#ifndef THRONG_TEST_UTIL_H
#define THRONG_TEST_UTIL_H

#include <thread>
#include <chrono>

// wait for a condition to become true because of an event in another
// thread. Executes 'stmt' after each wait iteration.
#define WAIT_FOR_DO_ONFAIL(condition, count, stmt, onfail) \
    {                                                      \
        int _c = 0;                                        \
        while (_c < count) {                               \
            if (condition) break;                          \
            _c += 1;                                       \
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); \
            stmt;                                          \
        }                                                  \
        BOOST_CHECK((condition));                          \
        if (!(condition))                                  \
            {onfail;}                                      \
    }

// wait for a condition to become true because of an event in another
// thread. Executes 'stmt' after each wait iteration.
#define WAIT_FOR_DO(condition, count, stmt) \
    WAIT_FOR_DO_ONFAIL(condition, count, stmt, ;)

// wait for a condition to become true because of an event in another
// thread
#define WAIT_FOR(condition, count)  WAIT_FOR_DO(condition, count, ;)

// wait for a condition to become true because of an event in another
// thread. Executes stmt
#define WAIT_FOR_ONFAIL(condition, count) \
    WAIT_FOR_DO_ONFAIL(condition, count, ;, onfail)

#endif /* THRONG_TEST_UTIL_H */
