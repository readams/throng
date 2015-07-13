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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "throng/ctx.h"
#include "temp_path.h"

#include <boost/test/unit_test.hpp>

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::string;
using std::unique_ptr;
using namespace throng;

BOOST_AUTO_TEST_SUITE(ctx_test)

BOOST_AUTO_TEST_CASE(connect) {
    throng::test::temp_dir storage;
    auto c1 = ctx::new_ctx((storage.path() / "n1").string());
    auto c2 = ctx::new_ctx((storage.path() / "n2").string());
    auto c3 = ctx::new_ctx((storage.path() / "n3").string());
    auto c4 = ctx::new_ctx((storage.path() / "n4").string());

    c1->register_store("test");
    c2->register_store("test");
    c3->register_store("test");
    c4->register_store("test");

    c1->configure_local({1,1}, "127.0.0.1", 17171);
    c2->configure_local({1,2}, "127.0.0.1", 17172);
    c3->configure_local({2,1}, "127.0.0.1", 17173);
    c4->configure_local({2,2}, "127.0.0.1", 17174);

    c2->add_seed("127.0.0.1", 17171);
    c3->add_seed("127.0.0.1", 17171);
    c4->add_seed("127.0.0.1", 17171);

    c1->start();
    c2->start();
    c3->start();
    c4->start();

    sleep(1);

    c1->stop();
    c2->stop();
    c3->stop();
    c4->stop();
}

BOOST_AUTO_TEST_SUITE_END()

