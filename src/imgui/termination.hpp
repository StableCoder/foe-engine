// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_TERMINATION_HPP
#define IMGUI_TERMINATION_HPP

class foeImGuiState;
struct ImGuiContext;

class foeImGuiTermination {
  public:
    bool terminationRequested() const noexcept;

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);

    void fileMainMenu();

    bool mTerminate{false};
};

#endif // IMGUI_TERMINATION_HPP