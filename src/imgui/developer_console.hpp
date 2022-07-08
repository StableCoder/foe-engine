// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_DEVELOPER_CONSOLE_HPP
#define IMGUI_DEVELOPER_CONSOLE_HPP

#include <foe/developer_console.hpp>

#include <string>

class foeImGuiState;
struct ImGuiContext;

class foeImGuiDeveloperConsole : public foeDeveloperConsole {
  public:
    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    std::string mBuffer;
    bool mOpen{false};
    bool mFocus{false};
};

#endif // IMGUI_DEVELOPER_CONSOLE_HPP