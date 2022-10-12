// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_SINK_HPP
#define FOE_LOG_SINK_HPP

#include <foe/export.h>
#include <foe/log/level.hpp>

#include <string_view>

class foeLogSink {
  public:
    virtual ~foeLogSink() = default;

    virtual void log(char const *pCategoryName, foeLogLevel level, std::string_view message) = 0;

    virtual void exception() = 0;
};

#endif // FOE_LOG_SINK_HPP