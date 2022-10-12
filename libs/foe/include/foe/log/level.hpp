// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_LOG_LEVEL_HPP
#define FOE_LOG_LEVEL_HPP

#include <foe/export.h>

#include <string>

enum foeLogLevel {
    Fatal = 0,
    Error,
    Warning,
    Info,
    Verbose,
    All = Verbose,
};

namespace std {
FOE_EXPORT std::string to_string(foeLogLevel logLevel);
}

#endif // FOE_LOG_LEVEL_HPP