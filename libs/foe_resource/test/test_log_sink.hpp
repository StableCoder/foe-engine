// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef TEST_LOG_SINK_HPP
#define TEST_LOG_SINK_HPP

#include <foe/log.hpp>

struct TestLogSink : public foeLogSink {
    void log(char const *, foeLogLevel level, std::string_view msg) {
        logMessages.emplace_back(LogEntry{
            .level = level,
            .msg = std::string{msg},
        });
    }
    void exception() {}

    struct LogEntry {
        foeLogLevel level;
        std::string msg;
    };

    std::vector<LogEntry> logMessages;
};

#endif // TEST_LOG_SINK_HPP