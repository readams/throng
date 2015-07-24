/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file singleton_task.h
 * @brief Interface definition file for singleton_task
 */
/*
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
#ifndef THRONG_SINGLETON_TASK_H
#define THRONG_SINGLETON_TASK_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <mutex>
#include <memory>
#include <chrono>

namespace throng {
namespace internal {

/**
 * A task that can be scheduled asynchronously from multiple threads
 * to run on an IO service but will ensure that only one copy is
 * running at a time.
 *
 * A singleton task that is rescheduled while it is currently running
 * will ensure that is restarted again after the current task finishes
 * running.
 */
class singleton_task {
public:
    /**
     * Create a new singleton task using the given IO service.  The
     * task will not be executed until the schedule method is invoked.
     *
     * @param io_service the IO service on which the task should run
     * @param task the task to run
     */
    singleton_task(boost::asio::io_service& io_service,
                   std::function<void()> task);
    ~singleton_task();

    /**
     * A duration for scheduling tasks
     */
    typedef std::chrono::milliseconds duration_type;

    /**
     * Schedule the task to run after some delay in the IO service
     * task pool.
     *
     * @param delay schedule the task to run after some delay.  A zero
     * duration means the task should run asynchronously immediately,
     * or after completion of a currently-executing instance.
     * @param max_delay If the task has been rescheduled without
     * running for longer than this interval, run the task anyway.
     * This can prevent starvation of the task if it is repeatedly
     * rescheduled.  If zero, then the task will not be checked for
     * starvation.
     */
    void schedule(duration_type delay = duration_type(0),
                  duration_type max_delay = duration_type(0));

    /**
     * Cancel the running task and do not reschedule it.
     */
    void cancel();

private:
    std::mutex task_mutex;
    std::function<void()> task;
    boost::asio::steady_timer task_timer;

    bool task_should_run = false;
    bool task_running = false;

    typedef std::chrono::steady_clock::time_point time_point;

    class task_instance
        : public std::enable_shared_from_this<task_instance> {
    public:
        task_instance(singleton_task& parent, time_point first_sched,
                      time_point next_sched);
        void run();
        void sched();

        singleton_task& parent;
        bool canceled = false;
        time_point first_sched;
        time_point next_sched;
    };

    std::shared_ptr<task_instance> instance;
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_SINGLETON_TASK_H */
