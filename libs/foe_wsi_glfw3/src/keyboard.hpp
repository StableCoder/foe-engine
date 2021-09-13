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

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <foe/wsi/keyboard.hpp>

void keyboardPreprocessing(foeWsiKeyboard *pKeyboard);

void keyCallback(foeWsiKeyboard *pKeyboard, int key, int, int action, int);

void charCallback(foeWsiKeyboard *pKeyboard, unsigned int codepoint);

#endif // KEYBOARD_HPP