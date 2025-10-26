// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef HID_KEYBOARD_HPP
#define HID_KEYBOARD_HPP

#include <cstdint>
#include <set>

struct KeyboardInput {
    /// Unicode character currently active
    uint32_t unicodeChar;
    // Only the last pressed key can be repeated
    uint32_t repeatKey;

    /// Set of keys that have received the 'pressed' signal.
    std::set<uint32_t> pressedKeys;
    /// Set of keys that have received the 'released' signal.
    std::set<uint32_t> releasedKeys;
    /// Set of keys currently held down.
    std::set<uint32_t> downKeys;

    /// Returns if the given key has just been pressed this tick.
    inline bool keyPressed(uint32_t key) const noexcept {
        return pressedKeys.find(key) != pressedKeys.end();
    }

    /// Returns if the given key has just been released.
    inline bool keyReleased(uint32_t key) const noexcept {
        return releasedKeys.find(key) != releasedKeys.end();
    }

    /// Returns if the given key is being held down.
    inline bool keyDown(uint32_t key) const noexcept {
        return downKeys.find(key) != downKeys.end();
    }

    void preprocessing() {
        unicodeChar = 0;
        repeatKey = 0;

        pressedKeys.clear();
        releasedKeys.clear();
    }
};

#endif // HID_KEYBOARD_HPP