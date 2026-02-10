// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_WINDOW_HPP
#define IMGUI_WINDOW_HPP

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"

#include <vector>

typedef char const *(*PFN_WindowBackend)(void *pContext);
typedef char const *(*PFN_WindowTitle)(void *pContext);
typedef bool (*PFN_WindowTerminationCalled)(void *pContext);
typedef void (*PFN_WindowSize)(void *pContext, int *pWidth, int *pHeight);
typedef void (*PFN_WindowContentScale)(void *pContext, float *pX, float *pY);

struct ImGuiContext;
class foeImGuiState;

class foeImGuiWindow {
  public:
    bool addWindow(void *pContext,
                   PFN_WindowBackend pfnBackend,
                   PFN_WindowTitle pfnTitle,
                   PFN_WindowTerminationCalled pfnTerminationCalled,
                   PFN_WindowSize pfnLogicalSize,
                   PFN_WindowSize pfnPixelSize,
                   PFN_WindowContentScale pfnContentScale,
                   KeyboardInput const *pKeyboard,
                   MouseInput const *pMouse);
    bool removeWindow(void *pContext);

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    struct WindowData {
        void *pContext;

        PFN_WindowBackend pfnBackend;
        PFN_WindowTitle pfnTitle;
        PFN_WindowTerminationCalled pfnTerminationCalled;
        PFN_WindowSize pfnLogicalSize;
        PFN_WindowSize pfnPixelSize;
        PFN_WindowContentScale pfnContentScale;

        KeyboardInput const *pKeyboard;
        MouseInput const *pMouse;

        bool open;
        bool focus;
    };

    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    std::vector<WindowData> mWindowList;
};

#endif // IMGUI_WINDOW_HPP