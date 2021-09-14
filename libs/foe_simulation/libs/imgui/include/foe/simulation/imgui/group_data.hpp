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

#include <foe/ecs/id.hpp>
#include <foe/imgui/base.hpp>
#include <foe/simulation/imgui/export.h>

struct foeSimulationState;

class foeSimulationImGuiGroupData : public foeImGuiBase {
  public:
    virtual ~foeSimulationImGuiGroupData() = default;

    FOE_SIM_IMGUI_EXPORT foeSimulationImGuiGroupData(foeSimulationState *pSimulationState);

  private:
    void viewMainMenu() final;
    void customUI() final;

    foeSimulationState *mpSimulationState;

    bool mOpen{false};
    bool mFocus{false};
    foeIdGroup mSelected{foeIdPersistentGroup};
};

#endif // FOE_SIMULATION_IMGUI_GROUP_DATA_HPP