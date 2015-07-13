/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file ctx_fixture.h
 * @brief Interface definition file for ctx_fixture
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
#ifndef THRONG_TEST_CTX_FIXTURE_H
#define THRONG_TEST_CTX_FIXTURE_H

#include "throng/ctx.h"
#include "throng/store_client.h"

#include "temp_path.h"

namespace throng {
namespace test {

/**
 * Test fixture for unit tests that require a library context
 */
class ctx_fixture {
public:
    ctx_fixture()
        : context(ctx::new_ctx(storage.path().string())) {
        context->configure_local({1}, "localhost", 17171);
        context->register_store("test");
        context->start();
    }
    virtual ~ctx_fixture() {
        context->stop();
    }

    temp_dir storage;
    std::unique_ptr<ctx> context;
};

/**
 * Adds a store client mapping string->string
 */
class string_store_fixture : public ctx_fixture {
public:
    string_store_fixture()
        : client(throng::store_client<std::string, std::string>::
                 new_store_client(*context, "test")) {
    }

    virtual ~string_store_fixture() {

    }

    std::unique_ptr<throng::store_client<std::string, std::string>> client;
};

} /* namespace test */
} /* namespace throng */

#endif /* THRONG_TEST_CTX_FIXTURE_H */
