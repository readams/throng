/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file temp_path.h
 * @brief Interface definition file for tmp_path
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
#ifndef THRONG_TEST_TEMP_PATH_H
#define THRONG_TEST_TEMP_PATH_H

#include "throng/ctx.h"
#include "throng/store_client.h"

#include <boost/filesystem.hpp>

namespace throng {
namespace test {

/**
 * Create a temporary directory that will be deleted recursively when
 * destructed
 */
class temp_dir {
public:
    temp_dir() {
        path_ = boost::filesystem::temp_directory_path();
        path_ /= "throng-test-";
        path_ += boost::filesystem::unique_path();
        boost::filesystem::create_directories(path_);
    }
    ~temp_dir() {
        boost::filesystem::remove_all(path_);
    }

    boost::filesystem::path& path() { return path_; };

private:
    boost::filesystem::path path_;
};

} /* namespace test */
} /* namespace throng */

#endif /* THRONG_TEST_TEMP_PATH_H */
