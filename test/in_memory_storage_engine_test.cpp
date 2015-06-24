/*
 * Test suite for in_memory_storage_engine
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

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(in_memory_storage_engine_test)

using throng::internal::in_memory_storage_engine;
using std::string;
using std::shared_ptr;
using std::make_shared;

BOOST_AUTO_TEST_CASE(basic) {
    in_memory_storage_engine e {"test"};

}

BOOST_AUTO_TEST_SUITE_END()
