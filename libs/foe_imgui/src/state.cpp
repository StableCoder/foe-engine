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

#include <foe/imgui/state.hpp>

#include <foe/imgui/base.hpp>
#include <imgui.h>

bool foeImGuiState::addUI(foeImGuiBase *pUI) {
    if (pUI == nullptr)
        return false;

    std::scoped_lock lock{mSync};

    // Check to see if its already in
    for (auto const *it : mUI) {
        if (it == pUI) {
            return false;
        }
    }

    mUI.emplace_back(pUI);

    return true;
}

void foeImGuiState::removeUI(foeImGuiBase *pUI) {
    std::scoped_lock lock{mSync};

    for (auto it = mUI.begin(); it != mUI.end(); ++it) {
        if (*it == pUI) {
            mUI.erase(it);
            return;
        }
    }
}

void foeImGuiState::runUI() {
    std::scoped_lock lock{mSync};

    ImGui::BeginMainMenuBar();

    // File menu
    if (ImGui::BeginMenu("File")) {
        for (auto *it : mUI) {
            it->fileMainMenu();
        }

        ImGui::EndMenu();
    }

    // View menu
    if (ImGui::BeginMenu("View")) {
        for (auto *it : mUI) {
            it->viewMainMenu();
        }

        ImGui::EndMenu();
    }

    // Custom menus
    for (auto *it : mUI) {
        it->customMainMenu();
    }

    ImGui::EndMainMenuBar();

    // Custom UI
    for (auto *it : mUI) {
        it->customUI();
    }
}