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
//! \cond
#define THRONG_LOGGER_H
//! \endcond

#include <sstream>
#include <iostream>
#include <string>

namespace throng {
namespace internal {

/**
 * Provides logging for the library
 */
class logger {
public:

    /**
     * Log levels
     */
    enum log_level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL,
        MAX_LEVEL
    };

    /**
     * A log message to be logged
     */
    struct log_message {
        /**
         * Destroy the message and log to the logger
         */
        ~log_message() {
            lgr.log(level_, file_, line_, function_, buffer_.str());
        }
        /**
         * Get a stream for appending to the message
         *
         * @return the stream
         */
        std::ostream& stream() { return buffer_; }

        /**
         * The logger associated with the message
         */
        logger& lgr;

        /**
         * The log level
         */
        log_level level_;
        /**
         * The filename for the message
         */
        char const* file_;
        /**
         * The line number for the message
         */
        int const line_;
        /**
         * The function name for the message
         */
        char const* function_;

        /**
         * The output stream for the message
         */
        std::ostringstream buffer_;
    };

    /**
     * Initialize a logger using the provided topic string
     *
     * @param topic_ the topic to use
     */
    explicit logger(const std::string topic_)
        : topic(std::move(topic_)) { }

    /**
     * Log a message to this logger
     *
     * @param level the level for the message
     * @param file the file for the message
     * @param line the line number for the message
     * @param function the name of the function that produced the message
     * @param message the actual message
     */
    void log(log_level level,
             char const* file,
             int const line,
             char const* function,
             const std::string& message);

    /**
     * Check whether a message a the given log level should be logged
     *
     * @param level the log level
     * @return true if the message should be logged
     */
    bool should_log(log_level level);

    /**
     * Get a string representation of the given log level
     *
     * @param level the log level
     * @return the human-readable string representation of the log
     * level.
     */
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
    virtual void log(logger::log_level level,
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
                            logger::log_level level) = 0;

    /**
     * Get the currently registered global sink
     *
     * @return the log sink
     */
    static log_sink& get_current_sink();
};

} /* namespace internal */
} /* namespace throng */

#endif /* THRONG_LOGGER_H */

/**
 * Allocate a new logger using the given topic as a subtopic of the
 * top-level throng topic.
 */
#define LOGGER(topic) \
    static ::throng::internal::logger lgr = \
        ::throng::internal::logger("throng." topic)

/**
 * Log a message to the given logger at the given log level
 */
#define LOGL(lgr, lvl)                                                  \
    if (lgr.should_log(::throng::internal::logger::lvl))                \
        ::throng::internal::logger::log_message{ lgr,                   \
                  ::throng::internal::logger::lvl,                      \
                  __FILE__, __LINE__, __FUNCTION__ }.stream()

/**
 * Log a message at the given log level assuming there is a logger in
 * scope named "lgr".
 */
#define LOG(lvl) LOGL(lgr, lvl)
