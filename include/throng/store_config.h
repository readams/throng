/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file ctx.h
 * @brief Interface definition file for throng ctx
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
#ifndef THRONG_STORE_CONFIG_H
#define THRONG_STORE_CONFIG_H

namespace throng {

/**
 * Configuration for a store
 */
struct store_config {
    /**
     * Set to true to enable persistence for this store
     */
    bool persistent = false;
};

} /* namespace throng */

#endif /* THRONG_STORE_CONFIG_H */
