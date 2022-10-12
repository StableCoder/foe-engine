// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_HPP
#define FOE_LOG_HPP

#include <foe/export.h>
#include <foe/log/logger.hpp>

/// Logs a compile-time message to the global logger with the given parameters
#define FOE_LOG(CATEGORY, LOG_LEVEL, MESSAGE, ...)                                                 \
    if (static_cast<int>(LOG_LEVEL) <= static_cast<int>(CATEGORY##LogCategoryGetLogLevel())) {     \
        foeLogger::instance()->log(CATEGORY##LogCategoryGetName(), LOG_LEVEL, MESSAGE,             \
                                   ##__VA_ARGS__);                                                 \
    }

/** Declares a log category for static or scoped environments
 * @param CATEGORY Name of the category, which is both how it appears in logs and used in the
 * function names
 */
#define FOE_DECLARE_LOG_CATEGORY(CATEGORY)                                                         \
                                                                                                   \
    char const *CATEGORY##LogCategoryGetName();                                                    \
                                                                                                   \
    foeLogLevel CATEGORY##LogCategoryGetLogLevel();                                                \
                                                                                                   \
    void CATEGORY##LogCategorySetLogLevel(foeLogLevel logLevel);

/** Declares a log category for shared environments
 * @param EXPORT Export macro to use for categories that are in shared binaries and shared
 * externally
 * @param CATEGORY Name of the category, which is both how it appears in logs and used in the
 * function names
 */
#define FOE_DECLARE_SHARED_LOG_CATEGORY(EXPORT)                                                    \
                                                                                                   \
    EXPORT char const *CATEGORY##LogCategoryGetName();                                             \
                                                                                                   \
    EXPORT foeLogLevel CATEGORY##LogCategoryGetLogLevel();                                         \
                                                                                                   \
    EXPORT void CATEGORY##LogCategorySetLogLevel(foeLogLevel logLevel);

/** Definition of a given log category, must appear in only a single compile unit
 * @param CATEGORY Name of the category, which is both how it appears in logs and used in the
 * function names
 * @param RUNTIME_DEFAULT_LEVEL Maximum log type accepted by the category at runtime by default
 */
#define FOE_DEFINE_LOG_CATEGORY(CATEGORY, RUNTIME_DEFAULT_LEVEL)                                   \
                                                                                                   \
    static foeLogLevel g_##CATEGORY##_log_level = RUNTIME_DEFAULT_LEVEL;                           \
                                                                                                   \
    char const *CATEGORY##LogCategoryGetName() { return #CATEGORY; }                               \
                                                                                                   \
    foeLogLevel CATEGORY##LogCategoryGetLogLevel() { return g_##CATEGORY##_log_level; }            \
                                                                                                   \
    void CATEGORY##LogCategorySetLogLevel(foeLogLevel logLevel) {                                  \
        g_##CATEGORY##_log_level = logLevel;                                                       \
    }

#endif // FOE_LOG_HPP