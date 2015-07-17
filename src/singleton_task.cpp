/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for ctx class.
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

#include "singleton_task.h"
#include "logger.h"

namespace throng {
namespace internal {

LOGGER("util");

using std::chrono::steady_clock;
using std::chrono::milliseconds;
using std::make_shared;
using boost::system::error_code;

singleton_task::singleton_task(boost::asio::io_service& io_service_,
                               std::function<void()> task)
    : io_service(io_service_), task(std::move(task)),
      task_timer(io_service_) { }
singleton_task::~singleton_task() {
    cancel();
}

singleton_task::task_instance::task_instance(singleton_task& parent_,
                                             time_point first_sched_,
                                             time_point next_sched_)
    : parent(parent_), first_sched(first_sched_),
      next_sched(next_sched_) { }

void singleton_task::task_instance::run() {
    {
        std::lock_guard<std::mutex> guard(parent.task_mutex);
        if (canceled || !parent.task_should_run)
            return;

        parent.task_running = true;
        parent.task_should_run = false;
    }
    try {
        parent.task();
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception while running task: "
                   << e.what();
    } catch (...) {
        LOG(ERROR) << "Unknown error while running task";
    }

    {
        std::lock_guard<std::mutex> guard(parent.task_mutex);

        parent.task_running = false;
        if (parent.task_should_run) {
            first_sched = steady_clock::now();
            sched();
        }
    }
}
void singleton_task::task_instance::sched() {
    auto self = shared_from_this();
    parent.task_timer.expires_at(next_sched);
    auto handler = [this, self](const error_code& ec) {
        if (!ec) run();
    };
    parent.task_timer.async_wait(handler);
}

void singleton_task::schedule(duration_type delay, duration_type max_delay) {
    {
        std::lock_guard<std::mutex> guard(task_mutex);
        bool need_queue = true;
        time_point now = steady_clock::now();
        time_point next_sched = now + delay;
        time_point first_sched = now;

        if (task_running || task_should_run) {
            if (task_running) {
                // reschedule task to run again after current instance
                // finishes
                instance->next_sched = next_sched;
                need_queue = false;
            } else if ((max_delay > duration_type(0)) &&
                       instance->first_sched + max_delay < next_sched) {
                // Task has been starved for longer than the maximum.
                // Allow it to run
                need_queue = false;
            } else {
                // cancel current queued task and create a new one
                instance->canceled = true;
                first_sched = instance->first_sched;
            }
        }

        task_should_run = true;

        if (need_queue) {
            instance = make_shared<task_instance>(*this,
                                                  first_sched, next_sched);
            instance->sched();
        }
    }
}

void singleton_task::cancel() {
    std::lock_guard<std::mutex> guard(task_mutex);
    if (task_should_run) {
        instance->canceled = true;
    }
    task_should_run = false;
    task_timer.cancel();
}

} /* namespace internal */
} /* namespace throng */
