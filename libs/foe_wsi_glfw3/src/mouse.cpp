// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <GLFW/glfw3.h>

#include "mouse.hpp"

void mousePreprocessing(foeWsiMouse *pMouse) {
    pMouse->oldInWindow = pMouse->inWindow;
    pMouse->oldPosition = pMouse->position;

    pMouse->scroll = {0, 0};

    pMouse->pressedButtons.clear();
    pMouse->releasedButtons.clear();
}

void cursorPositionCallback(foeWsiMouse *pMouse, double xPos, double yPos) {
    pMouse->position = {xPos, yPos};
}

void cursorEnterCallback(foeWsiMouse *pMouse, int entered) { pMouse->inWindow = entered; }

void scrollCallback(foeWsiMouse *pMouse, double xOffset, double yOffset) {
    pMouse->scroll = {xOffset, yOffset};
}

void buttonCallback(foeWsiMouse *pMouse, int button, int action, int) {
    if (action == GLFW_PRESS) {
        pMouse->pressedButtons.insert(button);
        pMouse->downButtons.insert(button);
    } else if (action == GLFW_RELEASE) {
        pMouse->releasedButtons.insert(button);
        pMouse->downButtons.erase(button);
    }
}