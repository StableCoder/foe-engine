// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WSI_GLFW_IMGUI_HPP
#define WSI_GLFW_IMGUI_HPP

#include <foe/external/imgui.h>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../imgui/window.hpp"

struct GLFW_WindowData;

bool imguiAddGlfwWindow(foeImGuiWindow *pImguiWindow,
                        GLFW_WindowData *pWindow,
                        KeyboardInput const *pKeyboard,
                        MouseInput const *pMouse);

ImGuiKey imguiGlfwKeyConvert(int keycode, int scancode);

#endif // WSI_GLFW_IMGUI_HPP