/*
 * Test suite for store_client
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

#include "ctx_fixture.h"

#include "throng/serializer_protobuf.h"
#include "throng_messages.pb.h"

#include <boost/test/unit_test.hpp>
#include <set>

THRONG_PROTOBUF_SERIALIZER(throng::message::node)

BOOST_AUTO_TEST_SUITE(store_client_test)

using std::string;
using std::vector;
using std::make_shared;
using throng::store_client;
using throng::versioned;
using throng::vector_clock;
using throng::node_id;
using throng::error::obsolete_version;

BOOST_FIXTURE_TEST_CASE(basic, throng::test::string_store_fixture) {
    auto v1 = client->get("hello");
    BOOST_CHECK(!v1);
    client->update("hello", v1, "world");

    auto v2 = client->get("hello");
    BOOST_REQUIRE(v2);
    BOOST_CHECK_EQUAL("world", v2.get());

    BOOST_CHECK_THROW(client->update("hello", v1, "world2"), obsolete_version);
    client->update("hello", v2, "world2");
    v2 = client->get("hello");
    BOOST_REQUIRE(v2);
    BOOST_CHECK_EQUAL("world2", v2.get());

    client->delete_key("hello", v2.get_version());
    v2 = client->get("hello");
    BOOST_REQUIRE(!v2);
}

BOOST_FIXTURE_TEST_CASE(inconsistency, throng::test::string_store_fixture) {
    auto union_resolver =
        [](vector<versioned<string>>& items) -> versioned<string> {
        std::set<char> chars;
        auto now = std::chrono::system_clock::now();
        throng::vector_clock maxClock { now, {} };
        for (auto& v : items) {
            if (v) {
                for (char c : v.get()) {
                    chars.insert(c);
                }
            }
            maxClock = maxClock.merge(v.get_version(), now);
        }
        std::vector<versioned<string>> result;
        std::stringstream str;
        for (char c : chars) {
            str << c;
        }
        return versioned<string>(make_shared<string>(str.str()),
                                 maxClock);
    };

    auto rclient =
        throng::store_client<string, string>::
        new_store_client(*context, "test", union_resolver);
    auto& raw = context->get_raw_store("test");

    node_id n1 = {1, 2, 3};
    node_id n2 = {1, 3, 2};
    node_id n3 = {2, 1, 4};
    auto now = std::chrono::system_clock::now();
    auto now1 = now + std::chrono::seconds(1);
    auto now2 = now + std::chrono::seconds(2);

    vector_clock v1 { now, { {n1, 1} } };
    vector_clock v2 { now1, { {n2, 2} } };
    vector_clock v3 { now2, { {n3, 3} } };

    raw.put("a", { std::make_shared<string>("abc"), v1 });
    raw.put("a", { std::make_shared<string>("def"), v2 });
    raw.put("a", { std::make_shared<string>("ghi"), v3 });

    auto val = client->get("a");
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL("ghi", val.get());
    vector_clock e { val.get_version().get_timestamp(),
            { {n1, 1}, {n2, 2}, {n3, 3} } };
    BOOST_CHECK_EQUAL(e, val.get_version());

    val = rclient->get("a");
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL("abcdefghi", val.get());

    vector_clock v4 { now2, { {n3, 2} } };
    raw.put("a", { std::make_shared<string>("jkl"), v3 });

    val = rclient->get("a");
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL("abcdefghi", val.get());
}

BOOST_FIXTURE_TEST_CASE(protobuf, throng::test::ctx_fixture) {
    using throng::message::node;

    auto c1 = store_client<string, node>::new_store_client(*context, "test");

    node n;
    auto id = n.mutable_id();
    id->add_id(5);
    id->add_id(4);
    id->add_id(3);
    n.set_hostname("127.0.0.1");
    n.set_port(1234);

    auto v1 = c1->get("hello");
    BOOST_CHECK(!v1);
    c1->update("hello", v1, n);

    auto v2 = c1->get("hello");
    BOOST_REQUIRE(v2);
    BOOST_CHECK_EQUAL(n.hostname(), v2.get().hostname());
    BOOST_CHECK_EQUAL(n.port(), v2.get().port());
}

BOOST_AUTO_TEST_SUITE_END()

