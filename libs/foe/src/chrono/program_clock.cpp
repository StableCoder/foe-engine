// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/chrono/program_clock.hpp>

namespace {
std::chrono::steady_clock::time_point cProgramStartTime = std::chrono::steady_clock::now();
}

auto foeProgramClock::now() noexcept -> time_point {
    return time_point(std::chrono::steady_clock::now() - cProgramStartTime);
}