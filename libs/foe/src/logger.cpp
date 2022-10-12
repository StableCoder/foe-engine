// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/log.h>

#include <mutex>
#include <string>
#include <vector>

namespace {

class foeLogger {
  public:
    foeLogger() = default;

    FOE_EXPORT void log(char const *pCategoryName, foeLogLevel level, char const *pMessage);

    FOE_EXPORT bool registerSink(void *pContext,
                                 PFN_foeLogMessage logMessage,
                                 PFN_foeLogException logException);

    FOE_EXPORT bool deregisterSink(void *pContext,
                                   PFN_foeLogMessage logMessage,
                                   PFN_foeLogException logException);

  private:
    std::mutex mSync;

    struct SinkSet {
        void *pContext;
        PFN_foeLogMessage logMessage;
        PFN_foeLogException logException;
    };
    std::vector<SinkSet> mSinks;
};

void foeLogger::log(char const *pCategoryName, foeLogLevel level, char const *pMessage) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mSinks) {
        if (it.logMessage)
            it.logMessage(it.pContext, pCategoryName, level, pMessage);
    }

    [[unlikely]] if (level == FOE_LOG_LEVEL_FATAL) {
        for (auto const &it : mSinks) {
            if (it.logException)
                it.logException(it.pContext);
        }
    }
}

bool foeLogger::registerSink(void *pContext,
                             PFN_foeLogMessage logMessage,
                             PFN_foeLogException logException) {
    std::scoped_lock lock{mSync};

    for (auto it = mSinks.begin(); it != mSinks.end(); ++it) {
        if ((pContext != nullptr && it->pContext == pContext) ||
            (pContext == nullptr && it->logMessage == logMessage &&
             it->logException == logException)) {
            return false;
        }
    }

    mSinks.emplace_back(SinkSet{
        .pContext = pContext,
        .logMessage = logMessage,
        .logException = logException,
    });
    return true;
}

bool foeLogger::deregisterSink(void *pContext,
                               PFN_foeLogMessage logMessage,
                               PFN_foeLogException logException) {
    std::scoped_lock lock{mSync};

    for (auto it = mSinks.begin(); it != mSinks.end(); ++it) {
        if ((pContext != nullptr && it->pContext == pContext) ||
            (pContext == nullptr && it->logMessage == logMessage &&
             it->logException == logException)) {
            mSinks.erase(it);
            return true;
        }
    }

    return false;
}

foeLogger logger;

} // namespace

extern "C" char const *foeLogLevel_to_string(foeLogLevel logLevel) {
    switch (logLevel) {
    case FOE_LOG_LEVEL_FATAL:
        return "Fatal";
    case FOE_LOG_LEVEL_ERROR:
        return "Error";
    case FOE_LOG_LEVEL_WARNING:
        return "Warning";
    case FOE_LOG_LEVEL_INFO:
        return "Info";
    case FOE_LOG_LEVEL_VERBOSE:
        return "Verbose";

    default:
        return "Unknown";
    }
}

extern "C" void foeLogMessage(char const *pCategoryName, foeLogLevel level, char const *pMessage) {
    logger.log(pCategoryName, level, pMessage);
}

extern "C" bool foeLogRegisterSink(void *pContext,
                                   PFN_foeLogMessage logMessage,
                                   PFN_foeLogException logException) {
    return logger.registerSink(pContext, logMessage, logException);
}

extern "C" bool foeLogDeregisterSink(void *pContext,
                                     PFN_foeLogMessage logMessage,
                                     PFN_foeLogException logException) {
    return logger.deregisterSink(pContext, logMessage, logException);
}