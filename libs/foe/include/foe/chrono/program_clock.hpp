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

#ifndef FOE_CHRONO_PROGRAM_CLOCK_HPP
#define FOE_CHRONO_PROGRAM_CLOCK_HPP

#include <foe/export.h>

#include <chrono>

struct foeProgramClock {
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<foeProgramClock, duration>;

    FOE_EXPORT static constexpr bool is_steady = true;
    FOE_EXPORT static auto now() noexcept -> time_point;
};

#endif // FOE_CHRONO_PROGRAM_CLOCK_HPP