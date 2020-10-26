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

/** A clock used when time dilation is necessary.
 * @note This clock is designed for use where it is updated every 'tick'. For a clock that can
 *     be updated infrequently, check out DilatedLongClock.
 *
 * The foeDilatedClock class type is used for accurate timekeeping using STL's std::chrono library
 * as a basis, and operates on the millisecond timescale.
 *
 * This class, is self-correcting, and allows for multipliers to be used against a real clock, and
 * self-corrects over time, even for small multipliers. This clock thus, can be used with time
 * multipliers of large magnitudes, or tiny ones.
 *
 * Periodic checks performed during clock updates prevents clock times from becoming desynchronized
 * from a fair degree of floating-point precision issues.
 */
class foeDilatedClock {
  public:
    /** Constructor
     * @param startTime The time the clock will start at.
     * @param timeDilation The starting time dilation value.
     */
    FOE_EXPORT explicit foeDilatedClock(
        std::chrono::nanoseconds startTime = std::chrono::nanoseconds{0}, float dilation = 1.f);

    /** Updated the internal clock based on the real time since the last update.
     * @param externalTime Current time of the external clock.
     * @param elapsed Elapsed time on the external clock since the last update.
     */
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

    /** Sets a new value for the internal clock
     * @param time The time for the internal clock to be set at.
     *
     * This also sets the elapsed time to 0, as if starting a fresh clock.
     */
    FOE_EXPORT void time(std::chrono::nanoseconds time) noexcept;

    /** Returns the current dilation value.
     * @return The time dilation value.
     */
    FOE_EXPORT float dilation() const noexcept;

    /** Sets a new time dilation value.
     * @param dilation Float value represnting the new time dilation rate.
     */
    FOE_EXPORT void dilation(float dilation) noexcept;

  protected:
    void resetChecks();

  private:
    struct ClockCheck {
        std::chrono::nanoseconds nextExternal;
        std::chrono::nanoseconds internalClockAtLast;
    };

    /// Times used for correcting time inaccuracies.
    /** The times at which foeDilatedClock check their time agains the real time, and ensure for
     * accurate timekeeping by correcting inaccuracies due to floating point issues.
     */
    static constexpr std::array<std::chrono::nanoseconds, 4> cCheckTimes{
        std::chrono::milliseconds(1000), std::chrono::milliseconds(500),
        std::chrono::milliseconds(100), std::chrono::milliseconds(50)};

    /// Current time on the internal clock.
    std::chrono::nanoseconds mInternalClock;
    /// The time that has elapsed on the clock since the last tick/update.
    std::chrono::nanoseconds mElapsedTime;
    /// The current time dilation, or value of time passes for this clock relative to real passing
    /// time.
    float mDilation;

    /// Set to true if the time checks should be reset/restarted.
    bool mResetChecks;
    /// An array of values that are used when the clock self-corrects
    std::array<ClockCheck, cCheckTimes.size()> mClockChecks;
};

#endif // FOE_CHRONO_DILATED_CLOCK_HPP