/*
    Copyright (C) 2020 George Cave.

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

#include <foe/log/logger.hpp>

#include <iostream>

foeLogger *foeLogger::instance() {
    static foeLogger gLogger;
    return &gLogger;
}

void foeLogger::log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message) {
    std::scoped_lock lock{mSync};

    for (auto *it : mSinks) {
        it->log(pCategory, level, message);
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