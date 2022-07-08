// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_WSI_IMGUI_WINDOW_HPP
#define FOE_WSI_IMGUI_WINDOW_HPP

#include <foe/wsi/imgui/export.h>
#include <foe/wsi/window.h>

#include <vector>

struct ImGuiContext;
class foeImGuiState;

class foeWsiImGuiWindow {
  public:
    FOE_WSI_IMGUI_EXPORT bool addWindow(foeWsiWindow window);
    FOE_WSI_IMGUI_EXPORT bool removeWindow(foeWsiWindow window);

    FOE_WSI_IMGUI_EXPORT bool registerUI(foeImGuiState *pState);
    FOE_WSI_IMGUI_EXPORT void deregisterUI(foeImGuiState *pState);

  private:
    struct WindowData {
        foeWsiWindow window;
        bool open;
        bool focus;
    };

    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    std::vector<WindowData> mWindowList;
};

#endif // FOE_WSI_IMGUI_WINDOW_HPP