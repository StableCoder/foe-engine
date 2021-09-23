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

#include "termination.hpp"

#include <foe/imgui/state.hpp>
#include <imgui.h>

#include <array>

namespace {

std::array<char const *, 1> renderMenus{
    "File",
};

}

bool foeImGuiTermination::terminationRequested() const noexcept { return mTerminate; }

bool foeImGuiTermination::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, &foeImGuiTermination::renderMenuElements, nullptr,
                         renderMenus.data(), renderMenus.size());
}

void foeImGuiTermination::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, &foeImGuiTermination::renderMenuElements, nullptr, renderMenus.data(),
                     renderMenus.size());
}

bool foeImGuiTermination::renderMenuElements(void *pContext, char const *pMenu) {
    auto *pData = static_cast<foeImGuiTermination *>(pContext);
    std::string_view menu{pMenu};

    if (menu == "File") {
        pData->fileMainMenu();
        return true;
    }

    return false;
}

void foeImGuiTermination::fileMainMenu() {
    if (ImGui::MenuItem("Exit")) {
        mTerminate = true;
    }
}