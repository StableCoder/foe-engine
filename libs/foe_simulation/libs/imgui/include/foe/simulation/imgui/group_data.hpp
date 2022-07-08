// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_IMGUI_GROUP_DATA_HPP
#define FOE_SIMULATION_IMGUI_GROUP_DATA_HPP

#include <foe/ecs/id.h>
#include <foe/simulation/imgui/export.h>

struct foeSimulation;
class foeImGuiState;
struct ImGuiContext;

class foeSimulationImGuiGroupData {
  public:
    FOE_SIM_IMGUI_EXPORT foeSimulationImGuiGroupData(foeSimulation *pSimulation);

    FOE_SIM_IMGUI_EXPORT bool registerUI(foeImGuiState *pState);
    FOE_SIM_IMGUI_EXPORT void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    foeSimulation *mpSimulationState;

    bool mOpen{false};
    bool mFocus{false};
    foeIdGroup mSelected{foeIdPersistentGroup};
};

#endif // FOE_SIMULATION_IMGUI_GROUP_DATA_HPP