// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_LOGGER_HPP
#define FOE_LOG_LOGGER_HPP

#include <fmt/core.h>
#include <foe/log/level.hpp>
#include <foe/log/sink.hpp>

#include <mutex>
#include <string_view>
#include <vector>

class foeLogger {
  public:
    FOE_EXPORT static foeLogger *instance();

    FOE_EXPORT void log(char const *pCategoryName, foeLogLevel level, std::string_view message);

    template <typename... Args>
    inline void log(char const *pCategoryName,
                    foeLogLevel level,
                    fmt::format_string<Args...> message,
                    Args &&...args) {
        log(pCategoryName, level, fmt::format(message, std::forward<Args>(args)...));
    }

    FOE_EXPORT bool registerSink(foeLogSink *pSink);

    FOE_EXPORT bool deregisterSink(foeLogSink *pSink);

  private:
    foeLogger();

    std::mutex mSync;
    std::vector<foeLogSink *> mSinks;
};

#endif // FOE_LOG_LOGGER_HPP