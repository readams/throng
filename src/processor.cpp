/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for processor class.
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

#include "processor.h"
#include "logger.h"

namespace throng {
namespace internal {

using std::vector;
using std::string;
using std::unique_ptr;
using std::chrono::steady_clock;
using std::chrono::milliseconds;
using boost::asio::steady_timer;

LOGGER("store");

vector<versioned<string>> processor::get(const string& key) {
    if (delegate) return delegate->get(key);

    std::lock_guard<std::mutex> guard(item_mutex);
    auto& key_index = item_map.get<key_tag>();
    auto kit = key_index.find(key);

    vector<versioned<string>> result;
    if (kit == key_index.end()) return result;
    for (auto& v : kit->details->values) {
        result.push_back(v);
    }
    return result;
}

void processor::start() {
    if (running) return;
    running = true;

    proc_timer =
        unique_ptr<steady_timer>(new steady_timer(ctx.get_io_service()));
    proc_timer->expires_from_now(std::chrono::milliseconds(500));
    proc_timer->async_wait(bind(&processor::on_proc_timer, this,
                                std::placeholders::_1));
}

void processor::stop() {
    if (running) {
        running = false;

        if (proc_timer)
            proc_timer->cancel();
    }
}

void processor::process(item_map_by_time::iterator& it) {

}

void processor::on_proc_timer(const boost::system::error_code& ec) {
    if (!running)
        return;

    auto now = steady_clock::now();
    static const time_point epoch;

    while (running) {
        std::lock_guard<std::mutex> guard(item_mutex);

        if (item_map.size() == 0) break;
        auto& next_index = item_map.get<next_time_tag>();

        auto it = next_index.begin();
        if (it->next_time != epoch && now < it->next_time) break;

        process(it);
    }

    proc_timer->expires_from_now(milliseconds(500));
    proc_timer->async_wait(bind(&processor::on_proc_timer, this,
                                std::placeholders::_1));
}

void processor::notify(const std::string& key, bool local) {
    for (auto& l : listeners) {
        try {
            l(key, local);
        } catch (...) {
        }
    }
}

bool processor::doput(item_details& rs,
                      const versioned<string>& value) {
    std::vector<versioned_t> new_values;

    for (auto vit = rs.values.begin(); vit != rs.values.end(); vit++) {
        vector_clock::occurred o =
            value.get_version().compare(vit->get_version());

        switch (o) {
        case vector_clock::occurred::BEFORE:
        case vector_clock::occurred::EQUAL:
            return false;
        case vector_clock::occurred::AFTER:
            break;
        default:
            new_values.push_back(*vit);
            break;
        }
    }
    new_values.push_back({ value });
    rs.values.swap(new_values);
    return true;
}

bool processor::put(const string& key,
                    const versioned<string>& value) {
    std::lock_guard<std::mutex> guard(item_mutex);
    auto& key_index = item_map.get<key_tag>();
    auto kit = key_index.find(key);

    time_point next_time;
    if (kit == key_index.end()) {
        auto r = key_index.insert(item(key, next_time));
        kit = r.first;
    }

    bool r = doput(*(kit->details), value);
    if (delegate && r) {
        r = delegate->put(key, value);
    }
    notify(key, true);
    return r;
}

const string& processor::get_name() const {
    if (delegate)
        return delegate->get_name();
    return name;
}

void processor::visit(store_visitor visitor) {
    std::lock_guard<std::mutex> guard(item_mutex);
    auto& key_index = item_map.get<key_tag>();
    for (auto& i : key_index) {
        visitor(i.key, i.details->values);
    }
}

} /* namespace internal */
} /* namespace throng */
