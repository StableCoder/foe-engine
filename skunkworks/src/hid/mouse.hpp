// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef HID_MOUSE_HPP
#define HID_MOUSE_HPP

#include <cstdint>
#include <set>

struct MouseInput {
    /// Holds the mouse cursor's X,Y coordinates
    struct Vec2 {
        double x;
        double y;
    };

    /// If the cursor is currently in the window or not.
    bool inWindow;
    /// If the mouse was in the window previously.
    bool oldInWindow;

    /// The current position of the mouse
    Vec2 position;
    /// The previous position of the mouse
    Vec2 oldPosition;

    /// The current mouse scroll wheel values
    Vec2 scroll;

    /// The set of mouse buttons just pressed.
    std::set<uint32_t> pressedButtons;
    /// The set of mouse buttons just released.
    std::set<uint32_t> releasedButtons;
    /// The set of mouse buttons currently being held down.
    std::set<uint32_t> downButtons;

    /// Returns if the mouse button was pressed
    inline bool buttonPressed(uint32_t button) const noexcept {
        return pressedButtons.find(button) != pressedButtons.end();
    }

    /// Returns if the mouse button was released
    inline bool buttonReleased(uint32_t button) const noexcept {
        return releasedButtons.find(button) != releasedButtons.end();
    }

    /// Returns if the mouse button is currently pressed down
    inline bool buttonDown(uint32_t button) const noexcept {
        return downButtons.find(button) != downButtons.end();
    }

    void preprocessing() {
        oldInWindow = inWindow;
        oldPosition = position;

        scroll = {0, 0};

        pressedButtons.clear();
        releasedButtons.clear();
    }
};

#endif // HID_MOUSE_HPP