// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_FRAME_TIME_INFO_HPP
#define IMGUI_FRAME_TIME_INFO_HPP

class FrameTimer;
class foeImGuiState;
struct ImGuiContext;

class foeImGuiFrameTimeInfo {
  public:
    foeImGuiFrameTimeInfo(FrameTimer *pFrameTimer);

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    FrameTimer const *const mFrameTimer;

    bool mOpen{true};
};

#endif // IMGUI_FRAME_TIME_INFO_HPP