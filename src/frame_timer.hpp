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

#ifndef FRAME_TIMER_HPP
#define FRAME_TIMER_HPP

#include <array>
#include <chrono>

class FrameTimer {
  public:
    /// Marks the current time as the beginning of a 'new' frame
    /** When this occurs, the time from the beginning of the last frame is recorded and added to the
     * array of previous recent frame times.
     */
    void newFrame();

    /// Returns current average frames per second based on last 128 frames
    auto framesPerSecond() const noexcept -> float {
        auto avg = averageFrameTime<std::chrono::nanoseconds>();
        return std::chrono::seconds(1) / avg;
    }

    /** Returns the time of the last frame
     * @tparam Duration Time scale to return as
     * @return Last time in `Duration` time scale
     */
    template <typename Duration>
    auto lastFrameTime() const noexcept -> Duration {
        return std::chrono::duration_cast<Duration>(mLastFrameTime);
    }

    /** Average time for the last 128 frames
     * @tparam Duration Time scale to return the time as
     * @return Average time in `Duration` tiem scale
     */
    template <typename Duration>
    auto averageFrameTime() const noexcept -> Duration {
        std::chrono::nanoseconds time{0};

        for (auto const &it : mTimeBetweenFrames) {
            time += it;
        }
        time /= mTimeBetweenFrames.size();

        return std::chrono::duration_cast<Duration>(time);
    }

  private:
    /// Length of time the last frame took
    std::chrono::nanoseconds mLastFrameTime{0};
    /// When the last frame was acquired successfully
    std::chrono::high_resolution_clock::time_point mLastFrameStart{
        std::chrono::high_resolution_clock::now()};
    /// Array of times between successfully acquired frames, to be used in round-robin fashion
    std::array<std::chrono::nanoseconds, 128> mTimeBetweenFrames{};
    /// Interator within the array for the current time to set
    std::array<std::chrono::nanoseconds, 128>::iterator mCurrentBetweenFrames{
        mTimeBetweenFrames.begin()};
};

#endif // FRAME_TIMER_HPP