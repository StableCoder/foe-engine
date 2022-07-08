// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_CATEGORY_HPP
#define FOE_LOG_CATEGORY_HPP

#include <foe/export.h>
#include <foe/log/level.hpp>

#include <string_view>

class foeLogCategory {
  public:
    FOE_EXPORT std::string_view name() const noexcept;

    FOE_EXPORT foeLogLevel maxLevel() const noexcept;
    FOE_EXPORT void maxLevel(foeLogLevel level) noexcept;

  protected:
    FOE_EXPORT
    foeLogCategory(std::string_view name, foeLogLevel maxLevel);

  private:
    const std::string_view mName;
    foeLogLevel mMaxLevel;
};

#endif // FOE_LOG_CATEGORY_HPP