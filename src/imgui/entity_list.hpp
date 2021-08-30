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

#include <foe/imgui/base.hpp>

struct foeSimulationState;

class foeImGuiEntityList : public foeImGuiBase {
  public:
    foeImGuiEntityList(foeSimulationState *pSimulationState);

  private:
    virtual void viewMainMenu();
    virtual void customUI();
    foeSimulationState *mpSimulationState;

    bool mOpen{false};
    bool mFocus{false};
};

#endif // IMGUI_ENTITY_LIST_HPP