/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for vector_clock class.
 *
 * Copyright (c) 2009 Webroot Software, Inc.
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

#include <iomanip>
#include <algorithm>

namespace throng {

using std::vector;

vector_clock::vector_clock()
    : timestamp(std::chrono::system_clock::now()) { }

vector_clock::vector_clock(time_point timestamp_,
                           const vector<clock_entry>& entries_)
    : timestamp(timestamp_), entries(entries_) {

}

vector_clock::vector_clock(time_point timestamp_,
                           const vector<clock_entry>&& entries_)
    : timestamp(timestamp_), entries(entries_) {

}

bool operator==(const vector_clock& l, const vector_clock& r) {
    return l.timestamp == r.timestamp && l.entries == r.entries;
}

bool operator!=(const vector_clock& l, const vector_clock& r) {
    return !(l == r);
}

std::ostream& operator<<(std::ostream& output, vector_clock::occurred o) {
    static const char* strs[] = { "AFTER", "BEFORE", "CONCURRENT", "EQUAL" };
    return output << strs[static_cast<size_t>(o)];
}

std::ostream& operator<<(std::ostream& output,
                         const node_id& id) {
    output << '(';
    bool first = true;
    for (auto& i : id) {
        if (!first)
            output << ",";
        first = false;
        output << i;
    }
    return output << ')';
}

std::ostream& operator<<(std::ostream& output,
                         const vector_clock::clock_entry& entry) {
    return output << "(" << entry.first << ", " << entry.second << ")";
}

std::ostream& operator<<(std::ostream& output,
                         const std::vector<vector_clock::clock_entry>& entries) {
    output << '[';
    bool first = true;
    for (auto& i : entries) {
        if (!first)
            output << ", ";
        first = false;
        output << i;
    }
    return output << "]";
}

std::ostream& operator<<(std::ostream& output, const vector_clock& ver) {
    return output << '{' << std::chrono::system_clock::to_time_t(ver.timestamp)
                  << ", " << ver.entries << '}';
}

vector_clock vector_clock::incremented(const node_id& id) const {
    return incremented(id, std::chrono::system_clock::now());
}

vector_clock vector_clock::incremented(const node_id& id,
                                       time_point timestamp) const {
    vector<clock_entry> newVersions(entries);
    size_t index = 0;
    bool found = false;
    for (; index < newVersions.size(); index++) {
        if (newVersions[index].first == id) {
            found = true;
            break;
        } else if (newVersions[index].first > id) {
            found = false;
            break;
        }
    }

    if (found) {
        newVersions[index].second += 1;
    } else {
        newVersions.emplace(newVersions.begin() + index, clock_entry{id, 1});
    }

    return vector_clock(timestamp, newVersions);
}

vector_clock vector_clock::merge(const vector_clock& o,
                                 time_point timestamp) const {
    vector<clock_entry> newVersions;
    size_t p1 = 0;
    size_t p2 = 0;

    while (p1 < entries.size() && p2 < o.entries.size()) {
        auto& ver1 = entries[p1];
        auto& ver2 = o.entries[p2];

        if (ver1.first == ver2.first) {
            newVersions.emplace_back(ver1.first,
                                     std::max(ver1.second, ver2.second));
            p1 += 1;
            p2 += 1;
        } else if (ver1.first < ver2.first) {
            newVersions.push_back(ver1);
            p1 += 1;
        } else {
            newVersions.push_back(ver2);
            p2 += 1;
        }
    }

    for (size_t i = p1; i < entries.size(); i++) {
        newVersions.push_back(entries[i]);
    }
    for (size_t i = p2; i < o.entries.size(); i++) {
        newVersions.push_back(o.entries[i]);
    }
    return vector_clock(timestamp, newVersions);
}

vector_clock vector_clock::merge(const vector_clock& o) const {
    return merge(o, std::chrono::system_clock::now());
}

vector_clock::occurred
vector_clock::compare(const vector_clock& o) const {
    bool c1bigger = false;
    bool c2bigger = false;
    size_t p1 = 0;
    size_t p2 = 0;

    while (p1 < entries.size() && p2 < o.entries.size()) {
        auto& ver1 = entries[p1];
        auto& ver2 = o.entries[p2];
        if (ver1.first == ver2.first) {
            if (ver1.second > ver2.second)
                c1bigger = true;
            else if (ver1.second < ver2.second)
                c2bigger = true;
            p1 += 1;
            p2 += 1;
        } else if (ver1.first > ver2.first) {
            c2bigger = true;
            p2 += 1;
        } else {
            c1bigger = true;
            p1 += 1;
        }
    }

    if (p1 < entries.size())
        c1bigger = true;
    else if (p2 < o.entries.size())
        c2bigger = true;

    if (!c1bigger && !c2bigger)
        return occurred::EQUAL;
    else if (!c1bigger && c2bigger)
        return occurred::BEFORE;
    else if (c1bigger && !c2bigger)
        return occurred::AFTER;
    return occurred::CONCURRENT;
}

} /* namespace throng */
