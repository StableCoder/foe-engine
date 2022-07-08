// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_RESOURCE_LIST_HPP
#define IMGUI_RESOURCE_LIST_HPP

#include <foe/ecs/id.h>

#include <vector>

class foeImGuiState;
struct foeSimulation;
class foeSimulationImGuiRegistrar;
struct ImGuiContext;

class foeImGuiResourceList {
  public:
    foeImGuiResourceList(foeSimulation *pSimulation, foeSimulationImGuiRegistrar *pRegistrar);

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    struct ResourceDisplayData {
        foeResourceID resourceID;
        bool open;
        bool focus;
    };

    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    void displayOpenResources();
    bool displayResource(ResourceDisplayData *pData);

    foeSimulation *mpSimulationState;
    foeSimulationImGuiRegistrar *mpRegistrar;

    bool mOpen{false};
    bool mFocus{false};

    std::vector<ResourceDisplayData> mDisplayed;
};

#endif // IMGUI_RESOURCE_LIST_HPP