// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_ENTITY_LIST_HPP
#define IMGUI_ENTITY_LIST_HPP

#include <foe/ecs/id.h>

#include <vector>

class foeImGuiState;
struct foeSimulation;
class foeSimulationImGuiRegistrar;
struct ImGuiContext;

class foeImGuiEntityList {
  public:
    foeImGuiEntityList(foeSimulation *pSimulation, foeSimulationImGuiRegistrar *pRegistrar);

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    struct EntityDisplayData {
        foeEntityID entity;
        bool open;
        bool focus;
    };

    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    void displayOpenEntities();
    bool displayEntity(EntityDisplayData *pData);

    foeSimulation *mpSimulationState;
    foeSimulationImGuiRegistrar *mpRegistrar;

    bool mOpen{false};
    bool mFocus{false};

    std::vector<EntityDisplayData> mDisplayed;
};

#endif // IMGUI_ENTITY_LIST_HPP