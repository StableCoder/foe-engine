// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WSI_QT_IMGUI_HPP
#define WSI_QT_IMGUI_HPP

#include <foe/external/imgui.h>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../imgui/window.hpp"

struct Qt_WindowData;

bool imguiAddQtWindow(foeImGuiWindow *pImguiWindow,
                      Qt_WindowData *pWindowData,
                      KeyboardInput const *pKeyboard,
                      MouseInput const *pMouse);

ImGuiKey imguiQtKeyConvert(int keycode, int scancode);

#endif // WSI_QT_IMGUI_HPP