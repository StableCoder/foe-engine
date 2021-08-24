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

#include "mouse.hpp"

void mousePreprocessing(foeMouse *pMouse) {
    pMouse->oldInWindow = pMouse->inWindow;
    pMouse->oldPosition = pMouse->position;

    pMouse->scroll = {0, 0};

    pMouse->pressedButtons.clear();
    pMouse->releasedButtons.clear();
}

void cursorPositionCallback(foeMouse *pMouse, double xPos, double yPos) {
    pMouse->position = {xPos, yPos};
}

void cursorEnterCallback(foeMouse *pMouse, int entered) { pMouse->inWindow = entered; }

void scrollCallback(foeMouse *pMouse, double xOffset, double yOffset) {
    pMouse->scroll = {xOffset, yOffset};
}

void buttonCallback(foeMouse *pMouse, int button, int action, int) {
    if (action == GLFW_PRESS) {
        pMouse->pressedButtons.insert(button);
        pMouse->downButtons.insert(button);
    } else if (action == GLFW_RELEASE) {
        pMouse->releasedButtons.insert(button);
        pMouse->downButtons.erase(button);
    }
}