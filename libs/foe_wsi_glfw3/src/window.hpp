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

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include <foe/wsi/window.hpp>

#include "keyboard.hpp"
#include "mouse.hpp"

#include <string>

struct foeWsiWindowGLFW {
    GLFWwindow *pWindow = nullptr;
    std::string title;
    bool resized;

    foeMouse mouse;
    foeKeyboard keyboard;
};

FOE_DEFINE_HANDLE_CASTS(window, foeWsiWindowGLFW, foeWsiWindow)

#endif // WINDOW_HPP