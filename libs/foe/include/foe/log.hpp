// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_HPP
#define FOE_LOG_HPP

#include <foe/export.h>
#include <foe/log/logger.hpp>

/// Logs a compile-time message to the global logger with the given parameters
#define FOE_LOG(CATEGORY, LOG_LEVEL, MESSAGE, ...)                                                 \
    if (static_cast<int>(foeLogLevel::LOG_LEVEL) <=                                                \
        static_cast<int>(foeLogCategoryGetLogLevel_##CATEGORY())) {                                \
        foeLogger::instance()->log(foeLogCategoryGetName_##CATEGORY(), foeLogLevel::LOG_LEVEL,     \
                                   MESSAGE, ##__VA_ARGS__);                                        \
    }

/** Declares a log category for static or scoped environments
 * @param CATEGORY Name of the category, which is both the class object and how it appears in logs
 */
#define FOE_DECLARE_LOG_CATEGORY(CATEGORY)                                                         \
                                                                                                   \
    char const *foeLogCategoryGetName_##CATEGORY();                                                \
                                                                                                   \
    foeLogLevel foeLogCategoryGetLogLevel_##CATEGORY();                                            \
                                                                                                   \
    void foeLogCategorySetLogLevel_##CATEGORY(foeLogLevel logLevel);

/** Declares a log category for shared environments
 * @param EXPORT Export macro to use for categories that are in shared binaries and shared
 * externally
 * @param CATEGORY Name of the category, which is both the class object and how it appears in logs
 */
#define FOE_DECLARE_SHARED_LOG_CATEGORY(EXPORT)                                                    \
                                                                                                   \
    EXPORT char const *foeLogCategoryGetName_##CATEGORY();                                         \
                                                                                                   \
    EXPORT foeLogLevel foeLogCategoryGetLogLevel_##CATEGORY();                                     \
                                                                                                   \
    EXPORT void foeLogCategorySetLogLevel_##CATEGORY(foeLogLevel logLevel);

/** Definition of a given log category, must appear in only a single compile unit
 * @param CATEGORY Name of the category, which is both the class object and how it appears in logs
 * @param RUNTIME_DEFAULT_LEVEL Maximum log type accepted by the category at runtime by default
 */
#define FOE_DEFINE_LOG_CATEGORY(CATEGORY, RUNTIME_DEFAULT_LEVEL)                                   \
                                                                                                   \
    static foeLogLevel gLogLevel_##CATEGORY = foeLogLevel::RUNTIME_DEFAULT_LEVEL;                  \
                                                                                                   \
    char const *foeLogCategoryGetName_##CATEGORY() { return #CATEGORY; }                           \
                                                                                                   \
    foeLogLevel foeLogCategoryGetLogLevel_##CATEGORY() { return gLogLevel_##CATEGORY; }            \
                                                                                                   \
    void foeLogCategorySetLogLevel_##CATEGORY(foeLogLevel logLevel) {                              \
        gLogLevel_##CATEGORY = logLevel;                                                           \
    }

#endif // FOE_LOG_HPP