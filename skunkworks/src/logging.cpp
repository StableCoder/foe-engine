// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "logging.hpp"

#include <foe/log.h>

#include <iostream>

namespace {

void log(void *, char const *pCategoryName, foeLogLevel level, char const *pMessage) {
    std::cout << foeLogLevel_to_string(level) << " : " << pCategoryName << " : " << pMessage
              << "\n";
}

void exception(void *) { std::cout << std::flush; }

} // namespace

void initializeLogging() { foeLogRegisterSink(nullptr, log, exception); }