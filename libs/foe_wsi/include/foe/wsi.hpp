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
    uint32_t unicodeChar;
    // Only the last pressed key can be repeated
    uint32_t repeatKey;

    std::set<uint32_t> pressedKeys;
    std::set<uint32_t> releasedKeys;
    std::set<uint32_t> heldKeys;
};

struct foeMouse {
    struct Vec2 {
        double x;
        double y;
    };

    bool inWindow;
    bool oldInWindow;

    Vec2 position;
    Vec2 oldPosition;

    Vec2 scroll;

    std::set<uint32_t> pressedButtons;
    std::set<uint32_t> releasedButtons;
    std::set<uint32_t> heldButtons;
};

FOE_WSI_EXPORT bool foeCreateWindow(int width, int height, const char *pTitle);
FOE_WSI_EXPORT void foeDestroyWindow();

FOE_WSI_EXPORT bool foeWindowShouldClose();
FOE_WSI_EXPORT void foeWindowEventProcessing();
FOE_WSI_EXPORT bool foeWindowResized();

FOE_WSI_EXPORT const foeMouse *foeGetMouse();
FOE_WSI_EXPORT const foeKeyboard *foeGetKeyboard();

#endif // FOE_WSI_HPP