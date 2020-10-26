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

#include <foe/chrono/dilated_clock.hpp>

using duration = std::chrono::nanoseconds;

foeDilatedClock::foeDilatedClock(duration startTime, float dilation) :
    mInternalClock{startTime},
    mElapsedTime{0},
    mDilation{dilation},
    mResetChecks{true},
    mClockChecks{} {}

void foeDilatedClock::update(duration externalTime, duration elapsed) noexcept {
    if (mDilation == 1.f) {
        mElapsedTime = elapsed;
        mInternalClock += elapsed;
        return;
    } else if (mDilation == 0.f) {
        mElapsedTime = duration(0);
        return;
    }

    // Time dilation active
    mElapsedTime = std::chrono::duration_cast<duration>(elapsed * mDilation);
    mInternalClock += mElapsedTime;

    bool modifiedTime = false;
    for (size_t i = 0; i < cCheckTimes.size(); ++i) {
        auto &clockCheck = mClockChecks[i];

        if (modifiedTime || mResetChecks) {
            // We modified a larger time period, so we're resetting the smaller timers
            clockCheck.internalClockAtLast = mInternalClock;
            clockCheck.nextExternal = externalTime + cCheckTimes[i];
        } else if (externalTime >= clockCheck.nextExternal) {
            // If we've exceeded the check time, perforce the accuracy check now.

            // Determine the true time elapsed total
            auto trueElapsedTime = std::chrono::duration_cast<duration>(
                externalTime - (clockCheck.nextExternal - cCheckTimes[i]));

            // Now, with the elapsed time, calculate how much dilated time should have passed
            auto calcElapsedTime =
                std::chrono::duration_cast<duration>(trueElapsedTime * mDilation);

            // Now calc the time it should be, according to this
            auto shouldBeTime = clockCheck.internalClockAtLast + calcElapsedTime;

            if (mInternalClock != shouldBeTime) {
                mInternalClock = shouldBeTime;
                modifiedTime = true;
            }

            clockCheck.internalClockAtLast = mInternalClock;
            clockCheck.nextExternal = externalTime + cCheckTimes[i];
        }
    }

    mResetChecks = false;
}

void foeDilatedClock::time(duration time) noexcept {
    mInternalClock = time;
    mElapsedTime = duration{0};
    mResetChecks = true;
}

float foeDilatedClock::dilation() const noexcept { return mDilation; }

void foeDilatedClock::dilation(float dilation) noexcept {
    mDilation = dilation;
    mResetChecks = true;
}

void foeDilatedClock::resetChecks() { mResetChecks = true; }