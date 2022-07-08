// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/log/level.hpp>

namespace std {

std::string to_string(foeLogLevel logLevel) {
#define ADD_LEVEL(X)                                                                               \
    case foeLogLevel::X:                                                                           \
        return #X;

    switch (logLevel) {
        ADD_LEVEL(Fatal)
        ADD_LEVEL(Error)
        ADD_LEVEL(Warning)
        ADD_LEVEL(Info)
        ADD_LEVEL(Verbose)

    default:
        return "Unknown Level";
    }

#undef ADD_LEVEL
}

} // namespace std