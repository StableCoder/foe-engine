// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/log/logger.hpp>

#include <iostream>

foeLogger *foeLogger::instance() {
    static foeLogger gLogger;
    return &gLogger;
}

void foeLogger::log(char const *pCategoryName, foeLogLevel level, std::string_view message) {
    std::scoped_lock lock{mSync};

    for (auto *it : mSinks) {
        it->log(pCategoryName, level, message);
    }

    [[unlikely]] if (level == foeLogLevel::Fatal) {
        for (auto *it : mSinks) {
            it->exception();
        }
    }
}

bool foeLogger::registerSink(foeLogSink *pSink) {
    std::scoped_lock lock{mSync};

    for (auto it = mSinks.begin(); it != mSinks.end(); ++it) {
        if (*it == pSink) {
            return false;
        }
    }

    mSinks.emplace_back(pSink);
    return true;
}

bool foeLogger::deregisterSink(foeLogSink *pSink) {
    std::scoped_lock lock{mSync};

    for (auto it = mSinks.begin(); it != mSinks.end(); ++it) {
        if (*it == pSink) {
            mSinks.erase(it);
            return true;
        }
    }

    return false;
}

foeLogger::foeLogger() = default;