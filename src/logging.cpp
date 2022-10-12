// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "logging.hpp"

#include <foe/log.hpp>

#include <iostream>

namespace {

class StdOutSink : public foeLogSink {
    void log(char const *pCategoryName, foeLogLevel level, std::string_view message) {
        std::cout << std::to_string(level) << " : " << pCategoryName << " : " << message << "\n";
    }

    void exception() { std::cout << std::flush; }
};

StdOutSink stdoutSink;

} // namespace

void initializeLogging() { foeLogger::instance()->registerSink(&stdoutSink); }