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

#include "test_util.h"
#include "logger.h"

#include "ctx_internal.h"
#include "rpc_service.h"
#include "temp_path.h"

#include <boost/test/unit_test.hpp>

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::string;
using std::vector;
using std::unique_ptr;
using namespace throng;

LOGGER("test.ctx");

BOOST_AUTO_TEST_SUITE(ctx_test)

BOOST_AUTO_TEST_CASE(connect) {
    throng::test::temp_dir storage;

    std::vector<node_id> nids { {1,1}, {1,2}, {2,1}, {2,2} };
    std::vector<std::unique_ptr<ctx>> ctxs;

    ctxs.push_back(ctx::new_ctx((storage.path() / "n1").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n2").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n3").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n4").string()));

    for (auto& c : ctxs)
        c->register_store("test");

    ctxs[0]->configure_local(nids[0], "127.0.0.1", 17171);
    ctxs[1]->configure_local(nids[1], "127.0.0.1", 17172);
    ctxs[2]->configure_local(nids[2], "127.0.0.1", 17173);
    ctxs[3]->configure_local(nids[3], "127.0.0.1", 17174);

    ctxs[1]->add_seed("127.0.0.1", 17171);
    ctxs[2]->add_seed("127.0.0.1", 17171);
    ctxs[3]->add_seed("127.0.0.1", 17171);

    for (auto& c : ctxs)
        c->start();

    auto check_ready = [&]() -> bool {
        for (auto& nid : nids) {
            if (nid == ctxs[0]->get_local_node_id()) continue;
            if (!dynamic_cast<internal::ctx_internal*>(ctxs[0].get())
                ->get_rpc_service().is_ready(nid)) {
                LOG(ERROR) << nid;
                return false;
            }
        }
        return true;
    };
    WAIT_FOR(check_ready(), 100);

    for (auto& c : ctxs)
        c->stop();
}

BOOST_AUTO_TEST_SUITE_END()

