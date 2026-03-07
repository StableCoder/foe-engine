// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_CHRONO_EASY_CLOCK_HPP
#define FOE_CHRONO_EASY_CLOCK_HPP

#include <chrono>

/** Class to easily re-use std::chrono clocks in looping code.
 * @tparam ClockType the std::chrono clock type to use as the underlying clock type (system_clock,
 * steady_clock or high_resolution_clock).
 *
 * Takes care of the boilerplate for when using a clock between two points, or within a looping
 * mechanism, storing the 'last' and 'current' time points for the requested clock type.
 *
 * Has functions for automatically casting to other chrono types (milliseconds, nanoseconds, etc).
 */
template <typename ClockType>
class foeEasyClock {
  public:
    /// Default constructor
    /** Sets both clock times to the current time for the templated ClockType (same as reset).
     */
    foeEasyClock() : mLastClockTime{ClockType::now()}, mCurrentClockTime{mLastClockTime} {}

    /// Updates the current time.
    /** Updates the current time, and sends the old 'current' time to be the last time.
     */
    void update() noexcept {
        mLastClockTime = mCurrentClockTime;
        mCurrentClockTime = ClockType::now();
    }

    /// Resets both clock times to ClockType::now()
    void reset() noexcept {
        mLastClockTime = ClockType::now();
        mCurrentClockTime = mLastClockTime;
    }

    /** Returns the elapsed time between the 'current' and 'last' times.
     * @tparam DurationType Chrono type to return.
     * @return Duration elapsed, casted to the return type.
     */
    template <typename DurationType>
    auto elapsed() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mCurrentClockTime - mLastClockTime);
    }

    /** Returns the time since epoch for the 'current' time.
     * @tparam DurationTypeChrono type to return.
     * @return Duration since epoch, casted to the return type.
     */
    template <typename DurationType>
    auto currentTime() const noexcept -> DurationType {
        return std::chrono::duration_cast<DurationType>(mCurrentClockTime.time_since_epoch());
    }

    /** Returns the 'last' time point.
     * @return The time point.
     */
    auto lastTimePoint() const noexcept -> typename ClockType::time_point { return mLastClockTime; }

    /** Returns the 'current' time point.
     * @return The time point.
     */
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