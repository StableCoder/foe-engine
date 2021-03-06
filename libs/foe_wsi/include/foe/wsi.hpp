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

#ifndef FOE_WSI_HPP
#define FOE_WSI_HPP

#include <foe/wsi_export.h>

#include <cstdint>
#include <set>

struct foeKeyboard {
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
};

struct foeMouse {
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

/** Creates a window for use
 * @param width Width of the new window
 * @param height Height of the new window
 * @param title The title for the new window
 * @return True if initialization successful, false otherwise, typically due to the window already
 * being active
 */
FOE_WSI_EXPORT bool foeCreateWindow(int width, int height, const char *pTitle, bool hide);

/// Destroys the current window
FOE_WSI_EXPORT void foeDestroyWindow();

/** Processes any waiting Window events
 *
 * This will move the current data to be old data, and check new state of keyboard and mouse.
 */
FOE_WSI_EXPORT void foeWindowEventProcessing();

FOE_WSI_EXPORT const char *foeWindowGetTitle();
FOE_WSI_EXPORT void foeWindowSetTitle(const char *pTitle);

FOE_WSI_EXPORT bool foeWindowGetShouldClose();
FOE_WSI_EXPORT void foeWindowSetShouldClose(bool terminate);

FOE_WSI_EXPORT void foeWindowGetSize(int *pWidth, int *pHeight);
FOE_WSI_EXPORT bool foeWindowResized();
FOE_WSI_EXPORT void foeWindowResize(int width, int height);

FOE_WSI_EXPORT void foeWindowHide();
FOE_WSI_EXPORT void foeWindowShow();

/** @brief
 * @param pScaleX X scaling factor requested for content
 * @param pScaleY Y scaling factor requested for content
 *
 * For macOS and Windows, content can be requested to be scaled independent of window sizing for
 * HiDPI screens.
 */
FOE_WSI_EXPORT void foeWindowGetContentScale(float *pScaleX, float *pScaleY);

FOE_WSI_EXPORT const foeMouse *foeGetMouse();
FOE_WSI_EXPORT const foeKeyboard *foeGetKeyboard();

#endif // FOE_WSI_HPP