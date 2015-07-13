/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for logger.
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

#include "logger.h"

#include <mutex>
#include <iostream>
#include <atomic>

namespace throng {
namespace internal {

const std::string logger::log_level_str[] =
    {"debug", "info", "warning", "error", "unknown"};

/**
 * A log sink for a std::ostream
 */
class log_sink_ostream : public log_sink {
public:
    log_sink_ostream(std::ostream& out_) : out(out_) { }
    virtual void log(log_level level,
                     const std::string& topic,
                     char const* file,
                     int const line,
                     char const* function,
                     const std::string& message) override {
        std::unique_lock<std::mutex> guard(lock);
        out << "[" << logger::level_str(level)
            << "] [" << topic
            << "] [" << file << ":" << line << ":" << function
            << "] " << message
            << std::endl;
    }
    virtual bool should_log(const std::string& topic,
                            log_level level) override {
        return true;
    }

private:
    std::mutex lock;
    std::ostream& out;
};

static std::atomic<log_sink*> current_log_sink(nullptr);

log_sink& log_sink::get_current_sink() {
    static log_sink_ostream default_log_sink(std::cout);
    log_sink* lsink = current_log_sink.load();
    if (lsink) return *lsink;
    return default_log_sink;
}

void logger::log(log_level level,
                 char const* file,
                 int const line,
                 char const* function,
                 const std::string& message) {
    log_sink::get_current_sink().log(level, topic, file,
                                     line, function, message);
}
bool logger::should_log(log_level level) {
    return log_sink::get_current_sink().should_log(topic, level);
}


} /* namespace internal */
} /* namespace throng */
