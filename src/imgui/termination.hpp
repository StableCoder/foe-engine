/*
    Copyright (C) 2020 George Cave.

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

#ifndef IMGUI_TERMINATION_HPP
#define IMGUI_TERMINATION_HPP

class foeImGuiState;

class foeImGuiTermination {
  public:
    bool terminationRequested() const noexcept;

    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

  private:
    static bool renderMenuElements(void *pContext, char const *pMenu);

    void fileMainMenu();

    bool mTerminate{false};
};

#endif // IMGUI_TERMINATION_HPP