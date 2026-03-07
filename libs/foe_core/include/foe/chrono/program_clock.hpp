// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CHRONO_PROGRAM_CLOCK_HPP
#define FOE_CHRONO_PROGRAM_CLOCK_HPP

#include <foe/chrono/easy_clock.hpp>
#include <foe/export.h>

#include <chrono>

/// Monotonic clock representing the amount of time passed since the program started
/** Class foeProgramClock represents a monotonic clock, based off the
 * std::chrono::steady_clock's time. The time points of this clock cannot decrease as physical time
 * moves forward. This clock is not related to wall clock time (for example, it can be time since
 * last reboot), and is most suitable for measuring intervals.
 *
 * `foeProgramClock` meets the requirements of `TrivialClock`.
 *
 * @warning As this clock relies on the 'start' time of the program, it is the actual start time of
 * the program if linked statically, or otherwise is the time the shared object/dll is loaded to the
 * program.
 */
struct foeProgramClock {
    /// arithmetic type representing the number of ticks in the clock's duration
    using duration = std::chrono::nanoseconds;
    /// a `std::ratio` type representing the tick period of the clock, in seconds
    using rep = duration::rep;
    /// `std::chrono::duration<rep, period>`
    using period = duration::period;
    /// `std::chrono::time_point<foeProgramClock>`
    using time_point = std::chrono::time_point<foeProgramClock, duration>;

    /// steady clock flag, always `true`
    FOE_EXPORT
    static constexpr bool is_steady = true;

    /// returns a time_point representing the current value of the clock
    FOE_EXPORT
    static auto now() noexcept -> time_point;
};

// Adds several nice-to-have, such as updating for ticks and elapsed time between those ticks.
using foeEasyProgramClock = foeEasyClock<foeProgramClock>;

#endif // FOE_CHRONO_PROGRAM_CLOCK_HPP