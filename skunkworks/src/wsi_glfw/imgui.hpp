// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WSI_GLFW_IMGUI_HPP
#define WSI_GLFW_IMGUI_HPP

#include <GLFW/glfw3.h>
#include <foe/external/imgui.h>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../imgui/window.hpp"

bool imguiAddGlfwWindow(foeImGuiWindow *pImguiWindow,
                        GLFWwindow *pWindow,
                        KeyboardInput const *pKeyboard,
                        MouseInput const *pMouse);

ImGuiKey imguiGlfwKeyConvert(int keycode);

#endif // WSI_GLFW_IMGUI_HPP