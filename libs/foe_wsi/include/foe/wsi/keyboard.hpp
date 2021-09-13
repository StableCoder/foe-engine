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

#ifndef FOE_WSI_KEYBOARD_HPP
#define FOE_WSI_KEYBOARD_HPP

#include <cstdint>
#include <set>

struct foeWsiKeyboard {
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

#endif // FOE_WSI_KEYBOARD_HPP