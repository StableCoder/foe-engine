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

/** A clock used for when time-dilation is necessary, with possibly multiple ticks between
 * updates.
 *
 * The DilatedLongClock operates the same as DilatedClock, but can be used in scenarios where the
 * clock is *not* updated every 'tick' or loop of the program.
 */
class foeDilatedLongClock : public foeDilatedClock {
  public:
    /** Constructor
     * @param externalClockTime The time of the external clock this clock will start at.
     * @param startTime The time the clock will start at.
     * @param timeDilation The starting time dilation value.
     */
    FOE_EXPORT explicit foeDilatedLongClock(
        std::chrono::nanoseconds externalTime,
        std::chrono::nanoseconds startTime = std::chrono::nanoseconds{0},
        float dilation = 1.f);

    /** Updates the clock.
     * @param externalTime The current time of the external clock used as a baseline.
     */
    FOE_EXPORT void update(std::chrono::nanoseconds externalTime) noexcept;

    /** Gets the value of the external clock that last time this object was updated
     * @return The external time this was last processed for/at.
     */
    FOE_EXPORT auto externalTime() const noexcept -> std::chrono::nanoseconds;

    /** Sets the clock's internally set external time, without performing other processing.
     * @param externalTime The external time to set.
     *
     * This function allows for changing the time that the object will continue counting from,
     * for example if there was a long pause or a change in the time stream for this clock, as in
     * switching which clock it is being based on with, for example, a different epoch.
     */
    FOE_EXPORT void externalTime(std::chrono::nanoseconds externalTime) noexcept;

  private:
    // Make the updateClock function from the base class non-public here.
    using foeDilatedClock::update;

    /// The last recorded external time this class was updated.
    std::chrono::nanoseconds mLastExternalTime;
};

#endif // FOE_CHRONO_DILATED_LONG_CLOCK_HPP