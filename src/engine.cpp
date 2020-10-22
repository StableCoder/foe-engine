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

#include <GLFW/glfw3.h>
#include <foe/wsi.hpp>

#include <iostream>

int main(int, char **) {
    foeCreateWindow(1280, 720, "FoE Engine");

    while (!foeWindowShouldClose()) {
        foeWindowEventProcessing();

        auto *pMouse = foeGetMouse();
        auto *pKeyboard = foeGetKeyboard();

        for (auto &it : pMouse->pressedButtons)
            std::cout << "Pressed: " << it << std::endl;
        for (auto &it : pMouse->releasedButtons)
            std::cout << "Released: " << it << std::endl;

        for (auto &it : pKeyboard->pressedKeys)
            std::cout << "Pressed: " << it << std::endl;
        for (auto &it : pKeyboard->releasedKeys)
            std::cout << "Released: " << it << std::endl;

        if (pKeyboard->unicodeChar != 0)
            std::cout << pKeyboard->unicodeChar << std::endl;

        if (pMouse->inWindow && !pMouse->oldInWindow) {
            std::cout << "Entered" << std::endl;
        } else if (!pMouse->inWindow && pMouse->oldInWindow) {
            std::cout << "Exited" << std::endl;
        }

        if (foeWindowResized())
            std::cout << "Resized" << std::endl;
    }

    foeDestroyWindow();

    return 0;
}