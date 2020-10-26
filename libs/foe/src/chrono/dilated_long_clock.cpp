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