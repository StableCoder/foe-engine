/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef TEST_LOG_SINK_HPP
#define TEST_LOG_SINK_HPP

#include <foe/log.hpp>

struct TestLogSink : public foeLogSink {
    void log(foeLogCategory *, foeLogLevel level, std::string_view msg) {
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