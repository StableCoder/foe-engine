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

#ifndef IMGUI_SAVE_HPP
#define IMGUI_SAVE_HPP

struct foeImGuiState;
struct foeSimulationState;

class foeImGuiSave {
  public:
    void setSimulationState(foeSimulationState *pSimulationState);
    void clearSimulationState();

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(void *pContext, char const *pMenuName);
    static void renderCustomUI(void *pContext);

    foeSimulationState *mpSimulationState{nullptr};
};

#endif // IMGUI_SAVE_HPP