// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include <foe/wsi/window.h>

#include "keyboard.hpp"
#include "mouse.hpp"

#include <string>

struct foeWsiWindowGLFW {
    GLFWwindow *pWindow = nullptr;
    std::string title;
    bool resized;

    foeWsiMouse mouse;
    foeWsiKeyboard keyboard;
};

FOE_DEFINE_HANDLE_CASTS(window, foeWsiWindowGLFW, foeWsiWindow)

#endif // WINDOW_HPP