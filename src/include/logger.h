/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*!
 * @file logger.h
 * @brief Interface definition file for throng logger
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
#ifndef THRONG_LOGGER_H
#define THRONG_LOGGER_H

#include <sstream>
#include <iostream>
#include <string>

namespace throng {
namespace internal {

enum log_level {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    MAX_LEVEL
};

/**
 * A log link that handles writing log messages to the final
 * destination
 */
class log_sink {
public:
    /**
     * Log a message to the sink.  This does not check whether the
     * current configuration would permit the log, so should be
     * preceded by a call to should_log.
     *
     * @param level the level for the message
     * @param topic the topic for the message
     * @param file the file for the message
     * @param line the line number for the message
     * @param function the name of the function that produced the message
     * @param message the actual message
     */
    virtual void log(log_level level,
                     const std::string& topic,
                     char const* file,
                     int const line,
                     char const* function,
                     const std::string& message) = 0;

    /**
     * Return true if this logger should emit a message for the given
     * topic and log level.
     *
     * @param topic the topic for a possible message
     * @param level the level for a possible message
     * @return true if a log message should be emitted
     */
    virtual bool should_log(const std::string& topic,
                            log_level level) = 0;

    /**
     * Get the currently registered global sink
     *
     * @return the log sink
     */
     static log_sink& get_current_sink();
};

/**
 * Provides logging for the library
 */
class logger {
public:
    explicit logger(const std::string topic_)
        : topic(std::move(topic_)) { }

    void log(log_level level,
             char const* file,
             int const line,
             char const* function,
             const std::string& message);
    bool should_log(log_level level);

    static const std::string& level_str(log_level level) {
        if (level >= log_level::DEBUG && level < log_level::MAX_LEVEL)
            return log_level_str[level];
        return log_level_str[log_level::MAX_LEVEL];
    }

private:
    const std::string topic;

    static const std::string log_level_str[];
};

/**
 * A log message to be logged
 */
struct log_message {
    ~log_message() {
        lgr.log(level_, file_, line_, function_, buffer_.str());
    }
    std::ostream& stream() { return buffer_; }

    logger& lgr;
    log_level level_;
    char const* file_;
    int const line_;
    char const* function_;

    std::ostringstream buffer_;
};

#define LOGGER(topic) logger("throng." topic)
#define LOGL(lgr, lvl)                                          \
    if (lgr.should_log(lvl))                                    \
        ::throng::internal::log_message{ lgr, lvl, __FILE__,    \
                  __LINE__, __FUNCTION__ }.stream()
#define LOG(lvl) LOGL(lgr, lvl)

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_LOGGER_H */
