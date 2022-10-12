// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef TEST_LOG_SINK_HPP
#define TEST_LOG_SINK_HPP

#include <foe/log.hpp>

struct TestLogSink {
    static void log(void *pContext, char const *, foeLogLevel level, char const *msg) {
        TestLogSink *pSink = (TestLogSink *)pContext;

        pSink->logMessages.emplace_back(LogEntry{
            .level = level,
            .msg = msg,
        });
    }

    struct LogEntry {
        foeLogLevel level;
        std::string msg;
    };

    std::vector<LogEntry> logMessages;
};

#endif // TEST_LOG_SINK_HPP