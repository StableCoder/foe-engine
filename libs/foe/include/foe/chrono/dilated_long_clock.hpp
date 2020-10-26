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

#ifndef FOE_CHRONO_DILATED_LONG_CLOCK_HPP
#define FOE_CHRONO_DILATED_LONG_CLOCK_HPP

#include <foe/chrono/dilated_clock.hpp>
#include <foe/export.h>

#include <chrono>

class foeDilatedLongClock : public foeDilatedClock {
  public:
    FOE_EXPORT explicit foeDilatedLongClock(
        std::chrono::nanoseconds externalTime,
        std::chrono::nanoseconds startTime = std::chrono::nanoseconds{0},
        float dilation = 1.f);

    FOE_EXPORT void update(std::chrono::nanoseconds externalTime) noexcept;

    FOE_EXPORT auto externalTime() const noexcept -> std::chrono::nanoseconds;
    FOE_EXPORT void externalTime(std::chrono::nanoseconds externalTime) noexcept;

  private:
    using foeDilatedClock::update;

    std::chrono::nanoseconds mLastExternalTime;
};

#endif // FOE_CHRONO_DILATED_LONG_CLOCK_HPP