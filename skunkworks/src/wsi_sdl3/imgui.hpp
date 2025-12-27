// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0
#ifndef WSI_SDL3_IMGUI_HPP
#define WSI_SDL3_IMGUI_HPP

#include <foe/external/imgui.h>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../imgui/window.hpp"
#include "window.hpp"

bool imguiAddSDL3Window(foeImGuiWindow *pImguiWindow,
                        SDL3_WindowData *pWindowData,
                        KeyboardInput const *pKeyboard,
                        MouseInput const *pMouse);

ImGuiKey imguiSDL3KeyConvert(int keycode, int scancode);

#endif // WSI_SDL3_IMGUI_HPP