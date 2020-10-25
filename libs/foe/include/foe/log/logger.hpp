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

#ifndef FOE_LOG_LOGGER_HPP
#define FOE_LOG_LOGGER_HPP

#include <fmt/core.h>
#include <foe/log/category.hpp>
#include <foe/log/level.hpp>
#include <foe/log/sink.hpp>

#include <mutex>
#include <string_view>

class foeLogger {
  public:
    FOE_EXPORT static foeLogger *instance();

    FOE_EXPORT void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message);

    template <typename... Args>
    inline void log(foeLogCategory *pCategory,
                    foeLogLevel level,
                    std::string_view message,
                    Args... args) {
        log(pCategory, level, fmt::format(message, args...));
    }

    FOE_EXPORT bool registerSink(foeLogSink *pSink);

    FOE_EXPORT bool deregisterSink(foeLogSink *pSink);

  private:
    foeLogger() = default;

    std::mutex mSync;
    std::vector<foeLogSink *> mSinks;
};

#endif // FOE_LOG_LOGGER_HPP