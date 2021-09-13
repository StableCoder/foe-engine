/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_WSI_MOUSE_HPP
#define FOE_WSI_MOUSE_HPP

#include <cstdint>
#include <set>

struct foeWsiMouse {
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
};

#endif // FOE_WSI_MOUSE_HPP