/*
 * Test suite for versioned
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "throng/versioned.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(versioned_test)

using std::string;
using throng::versioned;
using throng::vector_clock;
using std::shared_ptr;
using std::make_shared;

BOOST_AUTO_TEST_CASE(basic) {
    versioned<string> x { make_shared<string>("test"), {} };

    BOOST_CHECK(x);
    BOOST_CHECK_EQUAL("test", x.get());
    BOOST_CHECK_EQUAL("test", x.value_or("foo"));
    BOOST_CHECK_EQUAL(vector_clock(x.get_version().get_timestamp(), {}),
                      x.get_version());

    versioned<string> y { shared_ptr<string>(nullptr), {} };
    BOOST_CHECK(!y);
    BOOST_CHECK_EQUAL("foo", y.value_or("foo"));
}

BOOST_AUTO_TEST_SUITE_END()
