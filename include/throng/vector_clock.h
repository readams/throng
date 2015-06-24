/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file vector_clock.h
 * @brief Interface definition file for vector_clock
 */
/* Copyright (c) 2009 Webroot Software, Inc.
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

#pragma once
#ifndef THRONG_VECTOR_CLOCK_H
#define THRONG_VECTOR_CLOCK_H

#include <ostream>
#include <chrono>
#include <utility>
#include <vector>

namespace throng {

/**
 * A node ID for a node in the cluster. Node IDs are formatted
 * topologically as a list of shorts.  An ID such as [2,3,4,5] could
 * correspond to datacenter 2, pod 3, rack 4, node 5.  Nodes should be
 * arranged such that failures are less correlated when the shared
 * prefix is shorter.
 */
typedef std::vector<uint32_t> node_id;

/**
 * A vector clock represents a version in the database, and allows us
 * to determine whether multiple events in the system are causally
 * connected.  A vector clock uses a version for each node that is
 * incremented when that node modifies the value.  This means that
 * vector clocks are partially ordered: a clock could be before or
 * after another clock, or they could be incomparable.  Clocks that
 * cannot be compared correspond to concurrent updates to the same
 * value.
 */
class vector_clock {
public:
    /**
     * An entry in the vector clock consisting of a node ID and a
     * version number for the associated node.
     */
    typedef std::pair<node_id, uint64_t> clock_entry;

    /**
     * A timestamp
     */
    typedef std::chrono::system_clock::time_point time_point;

    /**
     * Default constructor
     */
    vector_clock() {}

    /**
     * Construct a vector_clock using the provided versions and
     * timestamp
     *
     * @param timestamp the timestamp to initialize to
     * @param entries the versions to include in the clock
     */
    vector_clock(time_point timestamp,
                 const std::vector<clock_entry>& entries);

    /**
     * Construct a vector_clock using the provided versions and
     * timestamp
     *
     * @param timestamp the timestamp to initialize to
     * @param entries the versions to include in the clock
     */
    vector_clock(time_point timestamp,
                 const std::vector<clock_entry>&& entries);

    /**
     * Possible vector_clock comparison values
     */
    enum class occurred : uint8_t {
        /** vector_clock 1 is after vector_clock 2 */
        AFTER,
        /** vector_clock 1 is before vector_clock 2 */
        BEFORE,
        /** vector_clock 1 and 2 are concurrent */
        CONCURRENT,
        /** vector_clock 1 and 2 are equal */
        EQUAL
    };

    /**
     * Increment the vector clock entry for the node and return a copy
     * of the clock with appropriate entry incremented.
     *
     * @param id the Node ID to increment
     * @param timestamp the new timestamp
     * @return the newly created vector clock
     */
    vector_clock incremented(const node_id& id, time_point timestamp) const;

    /**
     * Return whether or not the given vector_clock preceeded this one,
     * succeeded it, or is concurrant with it
     *
     * @param v The other vector_clock
     * @return one of the Occurred values
     */
    occurred compare(const vector_clock& v) const;

private:
    time_point timestamp;
    std::vector<clock_entry> entries;

    friend std::ostream& operator<<(std::ostream& output,
                                    const vector_clock& ver);
    friend bool operator==(const vector_clock& l, const vector_clock& r);
    friend bool operator!=(const vector_clock& l, const vector_clock& r);
};


/**
 * Stream insertion operator for a node ID.  Displays like "(1,2,3)"
 */
std::ostream& operator<<(std::ostream& output, const node_id& id);

/**
 * Stream insertion operator for vector:clock::occurred.  Prints the
 * constant name.
 */
std::ostream& operator<<(std::ostream& output, vector_clock::occurred o);

/**
 * Stream insertion operator for vector:clock_entry.
 * Prints as (node_id, version)
 */
std::ostream& operator<<(std::ostream& output,
                         const vector_clock::clock_entry& entry);

/**
 * Stream insertion operator for vector_clock
 */
std::ostream& operator<<(std::ostream& output, const vector_clock& ver);

/**
 * Check for vector clock equality
 */
bool operator==(const vector_clock& l, const vector_clock& r);
/**
 * Check for vector clock inequality
 */
bool operator!=(const vector_clock& l, const vector_clock& r);

} /* namespace throng */

#endif /* THRONG_VECTOR_CLOCK_H */
