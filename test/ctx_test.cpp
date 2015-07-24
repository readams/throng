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
#include "cluster_config.h"
#include "rpc_service.h"
#include "temp_path.h"

#include <boost/test/unit_test.hpp>

#include <vector>

using std::string;
using std::vector;
using std::unique_ptr;
using namespace throng;
using internal::node_client_t;

LOGGER("test.ctx");

BOOST_AUTO_TEST_SUITE(ctx_test)

BOOST_AUTO_TEST_CASE(connect) {
    throng::test::temp_dir storage;
    using internal::cluster_config_p;
    using internal::cluster_config;

    std::vector<node_id> nids { {1,1}, {1,2}, {2,1}, {2,2} };
    std::vector<std::unique_ptr<ctx>> ctxs;

    ctxs.push_back(ctx::new_ctx((storage.path() / "n1").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n2").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n3").string()));
    ctxs.push_back(ctx::new_ctx((storage.path() / "n4").string()));

    ctxs[0]->configure_local(nids[0], "127.0.0.1", 17171);
    ctxs[1]->configure_local(nids[1], "127.0.0.1", 17172, false);
    ctxs[2]->configure_local(nids[2], "127.0.0.1", 17173);
    ctxs[3]->configure_local(nids[3], "127.0.0.1", 17174, false);

    for (auto& c : ctxs) {
        c->register_store("test");
        auto nc = node_client_t::new_store_client(*c, internal::NODE_STORE);
        message::node n;
        n.set_hostname("127.0.0.1");

        for (size_t i = 0; i < ctxs.size(); i++) {
            if (ctxs[i].get() == c.get()) continue;

            n.set_port(17171 + i);
            n.set_master_eligible((i % 2) == 0);

            n.clear_id();
            auto id = n.mutable_id();
            for (auto i : c->get_local_node_id())
                id->add_id(i);
        }

        auto v = nc->get(n.id());
        nc->update(n.id(), v, n);
    }

    std::shared_ptr<cluster_config> config0(new cluster_config());
    std::shared_ptr<cluster_config> config1(new cluster_config());

    cluster_config::neigh_p neigh_(new message::neighborhood());
    neigh_->mutable_prefix();
    auto master = neigh_->add_masters();
    for (auto i : nids[0])
        master->add_id(i);
    master = neigh_->add_masters();
    for (auto i : nids[2])
        master->add_id(i);

    cluster_config::neigh_p neigh_0(new message::neighborhood());
    auto prefix = neigh_->mutable_prefix();
    prefix->add_id(0);
    master = neigh_->add_masters();
    for (auto i : nids[0])
        master->add_id(i);

    cluster_config::neigh_p neigh_1(new message::neighborhood());
    prefix = neigh_->mutable_prefix();
    prefix->add_id(1);
    master = neigh_->add_masters();
    for (auto i : nids[2])
        master->add_id(i);

    config0->add_neighborhood(neigh_);
    config0->add_neighborhood(neigh_1);
    config1->add_neighborhood(neigh_);
    config1->add_neighborhood(neigh_1);

    dynamic_cast<internal::ctx_internal*>(ctxs[0].get())
        ->set_static_config(config0);
    dynamic_cast<internal::ctx_internal*>(ctxs[1].get())
        ->set_static_config(config0);
    dynamic_cast<internal::ctx_internal*>(ctxs[2].get())
        ->set_static_config(config1);
    dynamic_cast<internal::ctx_internal*>(ctxs[3].get())
        ->set_static_config(config1);

    ctxs[1]->add_seed("127.0.0.1", 17171);
    ctxs[2]->add_seed("127.0.0.1", 17171);
    ctxs[3]->add_seed("127.0.0.1", 17171);

    for (auto& c : ctxs)
        c->start();

    auto check_ready = [&ctxs, &nids]() -> bool {
        for (auto& nid : nids) {
            if (nid == ctxs[0]->get_local_node_id()) continue;
            if (!dynamic_cast<internal::ctx_internal*>(ctxs[0].get())
                ->get_rpc_service().is_ready(nid)) {
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

