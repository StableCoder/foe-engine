// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "frame_timer.hpp"

void FrameTimer::newFrame() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    mLastFrameTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - mLastFrameStart);
    *mCurrentBetweenFrames = mLastFrameTime;

    mLastFrameStart = currentTime;

    ++mCurrentBetweenFrames;
    if (mCurrentBetweenFrames == mTimeBetweenFrames.end()) {
        mCurrentBetweenFrames = mTimeBetweenFrames.begin();
    }
}