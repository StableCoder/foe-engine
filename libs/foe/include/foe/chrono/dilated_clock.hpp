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

#ifndef FOE_CHRONO_DILATED_CLOCK_HPP
#define FOE_CHRONO_DILATED_CLOCK_HPP

#include <foe/export.h>

#include <array>
#include <chrono>

constexpr std::array<std::chrono::nanoseconds, 4> cCheckTimes{
    std::chrono::milliseconds(1000), std::chrono::milliseconds(500), std::chrono::milliseconds(100),
    std::chrono::milliseconds(50)};

class foeDilatedClock {
  public:
    FOE_EXPORT explicit foeDilatedClock(
        std::chrono::nanoseconds startTime = std::chrono::nanoseconds{0}, float dilation = 1.f);

    FOE_EXPORT void update(std::chrono::nanoseconds externalTime,
                           std::chrono::nanoseconds elapsed) noexcept;

    template <typename DurationType = std::chrono::nanoseconds>
    auto elapsed() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mElapsedTime);
    }

    template <typename DurationType = std::chrono::nanoseconds>
    auto time() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mInternalClock);
    }

    FOE_EXPORT void time(std::chrono::nanoseconds time) noexcept;
    FOE_EXPORT float dilation() const noexcept;
    FOE_EXPORT void dilation(float dilation) noexcept;

  protected:
    void resetChecks();

  private:
    std::chrono::nanoseconds mInternalClock;
    std::chrono::nanoseconds mElapsedTime;
    float mDilation;

    struct ClockCheck {
        std::chrono::nanoseconds nextExternal;
        std::chrono::nanoseconds internalClockAtLast;
    };

    bool mResetChecks;
    std::array<ClockCheck, cCheckTimes.size()> mClockChecks;
};

#endif // FOE_CHRONO_DILATED_CLOCK_HPP