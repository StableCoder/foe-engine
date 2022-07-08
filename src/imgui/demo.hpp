// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_DEMO_HPP
#define IMGUI_DEMO_HPP

struct ImGuiContext;
class foeImGuiState;

class foeImGuiDemo {
  public:
    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);
};

#endif // IMGUI_DEMO_HPP