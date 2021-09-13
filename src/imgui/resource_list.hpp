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

#ifndef IMGUI_RESOURCE_LIST_HPP
#define IMGUI_RESOURCE_LIST_HPP

#include <foe/ecs/id.hpp>
#include <foe/imgui/base.hpp>

#include <vector>

struct foeSimulationState;
class foeSimulationImGuiRegistrar;

class foeImGuiResourceList : public foeImGuiBase {
  public:
    foeImGuiResourceList(foeSimulationState *pSimulationState,
                         foeSimulationImGuiRegistrar *pRegistrar);

  private:
    struct ResourceDisplayData {
        foeResourceID resource;
        bool open;
        bool focus;
    };

    virtual void viewMainMenu();
    virtual void customUI();

    void displayOpenResources();
    bool displayResource(ResourceDisplayData *pData);

    foeSimulationState *mpSimulationState;
    foeSimulationImGuiRegistrar *mpRegistrar;

    bool mOpen{false};
    bool mFocus{false};

    std::vector<ResourceDisplayData> mDisplayed;
};

#endif // IMGUI_RESOURCE_LIST_HPP