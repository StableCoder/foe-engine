// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef HID_KEYBOARD_HPP
#define HID_KEYBOARD_HPP

#include <cstdint>
#include <vector>

struct KeyboardInput {
    struct CodePair {
        uint32_t keycode;
        uint32_t scancode;
    };

    /// Unicode character currently active
    uint32_t unicodeChar;
    // Only the last pressed key can be repeated
    CodePair repeatCode;

    /// Set of keycodes/scancodes that have received the 'pressed' signal.
    std::vector<CodePair> pressedCodes;
    /// Set of keycodes/scancodes that have received the 'released' signal.
    std::vector<CodePair> releasedCodes;
    /// Set of keycodes/scancodes currently held down.
    std::vector<CodePair> downCodes;

    /// returns if the given keycode has just been pressed
    inline bool keycodePressed(uint32_t keycode) const noexcept {
        for (auto const &it : pressedCodes) {
            if (it.keycode == keycode)
                return true;
        }

        return false;
    }

    /// returns if the given scancode has just been pressed
    inline bool scancodePressed(uint32_t scancode) const noexcept {
        for (auto const &it : pressedCodes) {
            if (it.scancode == scancode)
                return true;
        }

        return false;
    }

    /// returns if the given keycode has just been released
    inline bool keycodeReleased(uint32_t keycode) const noexcept {
        for (auto const &it : releasedCodes) {
            if (it.keycode == keycode)
                return true;
        }

        return false;
    }

    /// returns if the given scancode has just been released
    inline bool scancodeReleased(uint32_t scancode) const noexcept {
        for (auto const &it : releasedCodes) {
            if (it.scancode == scancode)
                return true;
        }

        return false;
    }

    /// returns if the given keycode is being held down
    inline bool keycodeDown(uint32_t keycode) const noexcept {
        for (auto const &it : downCodes) {
            if (it.keycode == keycode)
                return true;
        }

        return false;
    }

    /// returns if the given scancode is being held down
    inline bool scancodeDown(uint32_t scancode) const noexcept {
        for (auto const &it : downCodes) {
            if (it.scancode == scancode)
                return true;
        }

        return false;
    }

    void preprocessing() {
        unicodeChar = 0;
        repeatCode = {};

        pressedCodes.clear();
        releasedCodes.clear();
    }
};

#endif // HID_KEYBOARD_HPP