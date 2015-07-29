/*
 * Test suite for singleton_task
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

#include <boost/test/unit_test.hpp>

#include <thread>
#include <atomic>

using boost::asio::io_service;
using throng::internal::singleton_task;
using std::chrono::milliseconds;

LOGGER("test.singleton_task");

BOOST_AUTO_TEST_SUITE(singleton_task_test)

BOOST_AUTO_TEST_CASE(basic) {
    io_service io;
    bool check = false;
    singleton_task s(io, [&check]() { check = true; });
    s.schedule();
    io.run();
    BOOST_CHECK(check);
}

BOOST_AUTO_TEST_CASE(delay) {
    io_service io;
    std::atomic_uint_fast64_t run_count(0);
    singleton_task s(io, [&run_count]() { run_count += 1; });
    s.schedule(milliseconds(20));
    std::thread worker([&io]() { io.run(); });

    BOOST_CHECK_EQUAL(0, run_count.load());
    std::this_thread::sleep_for(milliseconds(30));
    BOOST_CHECK_EQUAL(1, run_count.load());

    worker.join();
    BOOST_CHECK_EQUAL(1, run_count.load());
}

BOOST_AUTO_TEST_CASE(cancel) {
    io_service io;
    std::atomic_uint_fast64_t run_count(0);
    singleton_task s(io, [&run_count]() { run_count += 1; });
    s.schedule(milliseconds(20));
    std::thread worker([&io]() { io.run(); });

    s.cancel();
    worker.join();
    BOOST_CHECK_EQUAL(0, run_count.load());
}

BOOST_AUTO_TEST_CASE(reschedule) {
    io_service io;

    std::atomic_uint_fast64_t run_count(0);
    singleton_task s(io, [&run_count]() { run_count += 1; });
    s.schedule(milliseconds(20));

    std::thread worker([&io]() { io.run(); });

    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(milliseconds(5));
        BOOST_CHECK_EQUAL(0, run_count.load());
        s.schedule(milliseconds(20));
    }
    worker.join();
    BOOST_CHECK_EQUAL(1, run_count.load());
}

BOOST_AUTO_TEST_CASE(reschedule_max) {
    io_service io;
    std::unique_ptr<io_service::work> work;
    work.reset(new io_service::work(io));

    std::atomic_uint_fast64_t run_count(0);
    singleton_task s(io, [&run_count]() { run_count += 1; });
    s.schedule(milliseconds(20), milliseconds(60));

    std::thread worker([&io]() { io.run(); });

    for (int i = 0; i < 18; i++) {
        std::this_thread::sleep_for(milliseconds(5));
        s.schedule(milliseconds(20), milliseconds(40));
    }

    work.reset();
    worker.join();
    BOOST_CHECK(3 <= run_count.load());
}

BOOST_AUTO_TEST_CASE(reschedule_while_running) {
    io_service io;
    std::unique_ptr<io_service::work> work;
    work.reset(new io_service::work(io));

    std::atomic_uint_fast64_t run_count(0);
    singleton_task s(io, [&run_count]() {
            std::this_thread::sleep_for(milliseconds(30));
            run_count += 1;
        });

    s.schedule(milliseconds(10));
    std::thread worker([&io]() { io.run(); });
    BOOST_CHECK_EQUAL(0, run_count.load());
    std::this_thread::sleep_for(milliseconds(30));
    s.schedule(milliseconds(10));

    work.reset();
    worker.join();
    BOOST_CHECK_EQUAL(2, run_count.load());
}

BOOST_AUTO_TEST_SUITE_END()
