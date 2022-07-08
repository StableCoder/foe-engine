// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <GLFW/glfw3.h>

#include "keyboard.hpp"

void keyboardPreprocessing(foeWsiKeyboard *pKeyboard) {
    pKeyboard->unicodeChar = 0;
    pKeyboard->repeatKey = 0;

    pKeyboard->pressedKeys.clear();
    pKeyboard->releasedKeys.clear();
}

void keyCallback(foeWsiKeyboard *pKeyboard, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
        pKeyboard->pressedKeys.insert(key);
        pKeyboard->downKeys.insert(key);
    } else if (action == GLFW_REPEAT) {
        pKeyboard->repeatKey = key;
    } else if (action == GLFW_RELEASE) {
        pKeyboard->releasedKeys.insert(key);
        pKeyboard->downKeys.erase(key);
    }
}

void charCallback(foeWsiKeyboard *pKeyboard, unsigned int codepoint) {
    pKeyboard->unicodeChar = codepoint;
}
