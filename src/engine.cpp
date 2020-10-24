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
#include <foe/graphics/environment.hpp>
#include <foe/log.hpp>
#include <foe/wsi.hpp>

#include <iostream>

int main(int, char **) {
    foeGfxEnvironment *pGfxEnvironment;
    auto res = foeGfxCreateEnvironment("FoE Engine", 0, &pGfxEnvironment);

    bool windowCreated = foeCreateWindow(1280, 720, "FoE Engine");

    FOE_LOG(General, Info, "Entering main loop")
    while (!foeWindowGetShouldClose()) {
        foeWindowEventProcessing();

        auto *pMouse = foeGetMouse();
        auto *pKeyboard = foeGetKeyboard();
    }
    FOE_LOG(General, Info, "Exiting main loop")

    foeDestroyWindow();

    foeGfxDestroyEnvironment(pGfxEnvironment);

    return 0;
}