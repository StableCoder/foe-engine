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

#ifndef FOE_CHRONO_EASY_CLOCK_HPP
#define FOE_CHRONO_EASY_CLOCK_HPP

#include <chrono>

template <typename ClockType>
class foeEasyClock {
  public:
    foeEasyClock() : mLastClockTime{ClockType::now()}, mCurrentClockTime{mLastClockTime} {}

    void update() noexcept {
        mLastClockTime = mCurrentClockTime;
        mCurrentClockTime = ClockType::now();
    }

    void reset() noexcept {
        mLastClockTime = ClockType::now();
        mCurrentClockTime = mLastClockTime;
    }

    template <typename DurationType>
    auto elapsed() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mCurrentClockTime - mLastClockTime);
    }

    template <typename DurationType>
    auto currentTime() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mCurrentClockTime.time_since_epoch());
    }

    auto lastTimePoint() const noexcept -> typename ClockType::time_point { return mLastClockTime; }

    auto currentTimePoint() const noexcept -> typename ClockType::time_point {
        return mCurrentClockTime;
    }

  private:
    // Second to last recorded time point
    typename ClockType::time_point mLastClockTime;
    // Last recorded time point
    typename ClockType::time_point mCurrentClockTime;
};

using foeEasySystemClock = foeEasyClock<std::chrono::system_clock>;
using foeEasySteadyClock = foeEasyClock<std::chrono::steady_clock>;
using foeEasyHighResClock = foeEasyClock<std::chrono::high_resolution_clock>;

#endif // FOE_CHRONO_EASY_CLOCK_HPP