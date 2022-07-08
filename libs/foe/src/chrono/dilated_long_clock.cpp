// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/chrono/dilated_long_clock.hpp>

foeDilatedLongClock::foeDilatedLongClock(std::chrono::nanoseconds externalTime,
                                         std::chrono::nanoseconds startTime,
                                         float dilation) :
    foeDilatedClock{startTime, dilation}, mLastExternalTime{externalTime} {}

void foeDilatedLongClock::update(std::chrono::nanoseconds externalTime) noexcept {
    // Calculated the elapsed time, send to the base class function
    auto elapsedTime = externalTime - mLastExternalTime;
    mLastExternalTime = externalTime;

    update(externalTime, elapsedTime);
}

auto foeDilatedLongClock::externalTime() const noexcept -> std::chrono::nanoseconds {
    return mLastExternalTime;
}

void foeDilatedLongClock::externalTime(std::chrono::nanoseconds externalTime) noexcept {
    mLastExternalTime = externalTime;
    resetChecks();
}