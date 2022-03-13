/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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