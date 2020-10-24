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

#ifndef FOE_LOG_HPP
#define FOE_LOG_HPP

#include <foe/export.h>
#include <foe/log/logger.hpp>

#define FOE_LOG(CATEGORY, LOG_LEVEL, MESSAGE)                                                      \
    if constexpr (static_cast<int>(foeLogLevel::LOG_LEVEL) <=                                      \
                  static_cast<int>(CATEGORY::maxCompileLevel())) {                                 \
        foeLogger::instance()->log(CATEGORY::instance(), foeLogLevel::LOG_LEVEL, MESSAGE);         \
    }

/** Declares a log category for static or scoped environments
 * @param CATEGORY Name of the category, which is both the class object and how it appears in logs
 * @param RUNTIME_DEFAULT_LEVEL Maximum log type accepted by the category at runtime by default
 * @param COMPILE_LEVEL Maximum log type that is compiled in.
 */
#define FOE_DECLARE_LOG_CATEGORY(CATEGORY, RUNTIME_DEFAULT_LEVEL, COMPILE_LEVEL)                   \
    class CATEGORY : public foeLogCategory {                                                       \
      public:                                                                                      \
        static foeLogCategory *instance() noexcept;                                                \
        constexpr static foeLogLevel maxCompileLevel() noexcept {                                  \
            return foeLogLevel::COMPILE_LEVEL;                                                     \
        }                                                                                          \
                                                                                                   \
      private:                                                                                     \
        CATEGORY(std::string_view category = #CATEGORY,                                            \
                 foeLogLevel maxLevel = foeLogLevel::RUNTIME_DEFAULT_LEVEL);                       \
    };

/** Declares a log category for shared environments
 * @param EXPORT Export macro to use for categories that are in shared binaries and shared
 * externally
 * @param CATEGORY Name of the category, which is both the class object and how it appears in logs
 * @param RUNTIME_DEFAULT_LEVEL Maximum log type accepted by the category at runtime by default
 * @param COMPILE_LEVEL Maximum log type that is compiled in.
 */
#define FOE_DECLARE_LOG_CATEGORY_SHARED(EXPORT, CATEGORY, RUNTIME_DEFAULT_LEVEL, COMPILE_LEVEL)    \
    class CATEGORY : public foeLogCategory {                                                       \
      public:                                                                                      \
        EXPORT static foeLogCategory *instance() noexcept;                                         \
        EXPORT constexpr static foeLogLevel maxCompileLevel() noexcept {                           \
            return foeLogLevel::COMPILE_LEVEL;                                                     \
        }                                                                                          \
                                                                                                   \
      private:                                                                                     \
        CATEGORY(std::string_view category = #CATEGORY,                                            \
                 foeLogLevel maxLevel = foeLogLevel::RUNTIME_DEFAULT_LEVEL);                       \
    };

/// Definition of a given log category, must appear in only a single compile unit
#define FOE_DEFINE_LOG_CATEGORY(CATEGORY)                                                          \
    foeLogCategory *CATEGORY::instance() noexcept {                                                \
        static CATEGORY gCategory;                                                                 \
        return &gCategory;                                                                         \
    }                                                                                              \
                                                                                                   \
    CATEGORY::CATEGORY(std::string_view category, foeLogLevel maxLevel) :                          \
        foeLogCategory{category, maxLevel} {}

/// Universal 'general' log for non-specific use.
FOE_DECLARE_LOG_CATEGORY_SHARED(FOE_EXPORT, General, All, All)

#endif // FOE_LOG_HPP