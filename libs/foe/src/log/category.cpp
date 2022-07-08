// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/log/category.hpp>

std::string_view foeLogCategory::name() const noexcept { return mName; }

foeLogLevel foeLogCategory::maxLevel() const noexcept { return mMaxLevel; }

void foeLogCategory::maxLevel(foeLogLevel level) noexcept { mMaxLevel = level; }

foeLogCategory::foeLogCategory(std::string_view name, foeLogLevel maxLevel) :
    mName{name}, mMaxLevel{maxLevel} {}