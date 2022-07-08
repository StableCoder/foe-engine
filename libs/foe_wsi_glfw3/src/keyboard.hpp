// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <foe/wsi/keyboard.hpp>

void keyboardPreprocessing(foeWsiKeyboard *pKeyboard);

void keyCallback(foeWsiKeyboard *pKeyboard, int key, int, int action, int);

void charCallback(foeWsiKeyboard *pKeyboard, unsigned int codepoint);

#endif // KEYBOARD_HPP