// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_LOGGER_HPP
#define FOE_LOG_LOGGER_HPP

#include <fmt/core.h>
#include <foe/log/level.hpp>

#include <mutex>
#include <vector>

typedef void (*PFN_foeLogMessage)(void *pContext,
                                  char const *pCategoryName,
                                  foeLogLevel level,
                                  char const *pMessage);
typedef void (*PFN_foeLogException)(void *pContext);

class foeLogger {
  public:
    FOE_EXPORT static foeLogger *instance();

    FOE_EXPORT void log(char const *pCategoryName, foeLogLevel level, char const *pMessage);

    template <typename... Args>
    inline void log(char const *pCategoryName,
                    foeLogLevel level,
                    fmt::format_string<Args...> message,
                    Args &&...args) {
        log(pCategoryName, level, fmt::format(message, std::forward<Args>(args)...).c_str());
    }

    FOE_EXPORT bool registerSink(void *pContext,
                                 PFN_foeLogMessage logMessage,
                                 PFN_foeLogException logException);

    FOE_EXPORT bool deregisterSink(void *pContext,
                                   PFN_foeLogMessage logMessage,
                                   PFN_foeLogException logException);

  private:
    foeLogger();

    std::mutex mSync;

    struct SinkSet {
        void *pContext;
        PFN_foeLogMessage logMessage;
        PFN_foeLogException logException;
    };
    std::vector<SinkSet> mSinks;
};

#endif // FOE_LOG_LOGGER_HPP