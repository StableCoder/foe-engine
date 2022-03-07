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

#ifndef IMGUI_ENTITY_LIST_HPP
#define IMGUI_ENTITY_LIST_HPP

#include <foe/ecs/id.hpp>

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