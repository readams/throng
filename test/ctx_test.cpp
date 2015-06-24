/*
 * Test suite for ctx
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

#include "throng/ctx.h"
#include "throng/serializer_protobuf.h"
#include "throng_messages.pb.h"

#include <boost/test/unit_test.hpp>

THRONG_PROTOBUF_SERIALIZER(throng::message::node)

BOOST_AUTO_TEST_SUITE(ctx_test)

using std::string;
using namespace throng;

BOOST_AUTO_TEST_CASE(basic) {
    ctx c {"/tmp/test"};

    c.register_store("test");
    c.register_store("test2", store_config{});
    c.start();
    c.stop();

    auto client = c.get_store_client<string, string>("test");

    auto client_pb = c.get_store_client<string, message::node>("test2");
}

BOOST_AUTO_TEST_SUITE_END()

