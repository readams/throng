/*
 * Test suite for vector_clock
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

#include "throng/vector_clock.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(vector_clock_test)

using throng::vector_clock;

BOOST_AUTO_TEST_CASE(increment) {
    auto now = std::chrono::system_clock::now();
    throng::node_id n1 = {1, 2, 3};
    throng::node_id n2 = {2, 1, 4};
    throng::node_id n3 = {1, 3, 2};
    throng::node_id n4 = {1, 2, 1};

    vector_clock e { now, { {n1, 1} } };
    vector_clock v = vector_clock().incremented(n1, now);
    BOOST_CHECK_EQUAL(e, v);

    e = { now, { {n1, 1}, {n2, 1} } };
    v = v.incremented(n2, now);
    BOOST_CHECK_EQUAL(e, v);

    e = { now, { {n1, 1}, {n2, 2} } };
    v = v.incremented(n2, now);
    BOOST_CHECK_EQUAL(e, v);

    e = { now, { {n1, 2}, {n2, 2} } };
    v = v.incremented(n1, now);
    BOOST_CHECK_EQUAL(e, v);

    e = { now, { {n1, 2}, {n3, 1}, {n2, 2} } };
    v = v.incremented(n3, now);
    BOOST_CHECK_EQUAL(e, v);

    e = { now, { {n4, 1}, {n1, 2}, {n3, 1}, {n2, 2} } };
    v = v.incremented(n4, now);
    BOOST_CHECK_EQUAL(e, v);
}

BOOST_AUTO_TEST_CASE(compare) {
    auto now = std::chrono::system_clock::now();
    throng::node_id n1 = {1, 2, 3};
    throng::node_id n2 = {1, 3, 2};
    throng::node_id n3 = {2, 1, 4};

    vector_clock v1;
    BOOST_CHECK_EQUAL(vector_clock::occurred::EQUAL, v1.compare(v1));

    v1 = {now, { {n1, 2}, {n2, 1} } };
    BOOST_CHECK_EQUAL(vector_clock::occurred::EQUAL, v1.compare(v1));

    vector_clock v2 { now, { {n1, 2}, {n2, 1}, {n3, 1} } };
    BOOST_CHECK_EQUAL(vector_clock::occurred::BEFORE, v1.compare(v2));
    BOOST_CHECK_EQUAL(vector_clock::occurred::AFTER, v2.compare(v1));
    BOOST_CHECK_EQUAL(vector_clock::occurred::AFTER, v1.compare(vector_clock()));

    v1 = { now, { {n1, 2} } };
    v2 = { now, { {n2, 1} } };
    BOOST_CHECK_EQUAL(vector_clock::occurred::CONCURRENT, v1.compare(v2));

    v1 = { now, { {n1, 3}, {n2, 2}, {n3, 1} } };
    v2 = { now, { {n1, 2}, {n2, 1}, {n3, 1} } };
    BOOST_CHECK_EQUAL(vector_clock::occurred::AFTER, v1.compare(v2));

    v1 = { now, { {n1, 3}, {n2, 2}, {n3, 1} } };
    v2 = { now, { {n1, 3}, {n2, 3}, {n3, 1} } };
    BOOST_CHECK_EQUAL(vector_clock::occurred::BEFORE, v1.compare(v2));
}

BOOST_AUTO_TEST_CASE(merge) {
    using namespace throng;
    auto now = std::chrono::system_clock::now();
    node_id n1 = {1, 2, 3};
    node_id n2 = {1, 3, 2};
    node_id n3 = {2, 1, 4};
    node_id n4 = {2, 2, 4};
    node_id n5 = {2, 2, 5};

    vector_clock e  { now, {} };
    vector_clock v1 { now, {} };
    vector_clock v2 { now, {} };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));

    e  = { now, { {n1, 1} } };
    v1 = { now, { {n1, 1} } };
    v2 = { now, { {n1, 1} } };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));

    e  = { now, { {n1, 1}, {n2, 1} } };
    v1 = { now, { {n2, 1} } };
    v2 = { now, { {n1, 1} } };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));

    e  = { now, { {n1, 2} } };
    v1 = { now, { {n1, 2} } };
    v2 = { now, { {n1, 1} } };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));

    e  = { now, { {n1, 2} } };
    v1 = { now, { {n1, 1} } };
    v2 = { now, { {n1, 2} } };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));

    e  = { now, { {n1, 3}, {n2, 2}, {n3, 1}, {n4, 1}, {n5, 1} } };
    v1 = { now, { {n1, 3}, {n2, 1}, {n3, 1}, {n5, 1} } };
    v2 = { now, { {n1, 1}, {n2, 2}, {n4, 1} } };
    BOOST_CHECK_EQUAL(e, v1.merge(v2, now));
}

BOOST_AUTO_TEST_SUITE_END()
