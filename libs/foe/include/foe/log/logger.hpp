// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_LOGGER_HPP
#define FOE_LOG_LOGGER_HPP

#include <fmt/core.h>
#include <foe/log/category.hpp>
#include <foe/log/level.hpp>
#include <foe/log/sink.hpp>

#include <mutex>
#include <string_view>
#include <vector>

class foeLogger {
  public:
    FOE_EXPORT static foeLogger *instance();

    FOE_EXPORT void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message);

#if FMT_VERSION >= 80000
    /// libfmt v8 is incompatible with v7, need this split.
    template <typename... Args>
    inline void log(foeLogCategory *pCategory,
                    foeLogLevel level,
                    fmt::format_string<Args...> message,
                    Args &&...args) {
        log(pCategory, level, fmt::format(message, std::forward<Args>(args)...));
    }
#else
    template <typename... Args>
    inline void log(foeLogCategory *pCategory,
                    foeLogLevel level,
                    std::string_view message,
                    Args &&...args) {
        log(pCategory, level, fmt::format(message, std::forward<Args>(args)...));
    }
#endif

    FOE_EXPORT bool registerSink(foeLogSink *pSink);

    FOE_EXPORT bool deregisterSink(foeLogSink *pSink);

  private:
    foeLogger();

    std::mutex mSync;
    std::vector<foeLogSink *> mSinks;
};

#endif // FOE_LOG_LOGGER_HPP