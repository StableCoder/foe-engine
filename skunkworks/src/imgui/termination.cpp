// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "termination.hpp"

#include <foe/external/imgui.h>
#include <foe/imgui/state.hpp>

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

bool foeImGuiTermination::renderMenuElements(ImGuiContext *pImGuiContext,
                                             void *pUserData,
                                             char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiTermination *>(pUserData);
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