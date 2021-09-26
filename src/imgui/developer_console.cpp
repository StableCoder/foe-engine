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

#include "developer_console.hpp"

#include <foe/imgui/state.hpp>
#include <imgui.h>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

}

bool foeImGuiDeveloperConsole::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiDeveloperConsole::renderMenuElements,
                         foeImGuiDeveloperConsole::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeImGuiDeveloperConsole::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiDeveloperConsole::renderMenuElements,
                     foeImGuiDeveloperConsole::renderCustomUI, renderMenus.data(),
                     renderMenus.size());
}

bool foeImGuiDeveloperConsole::renderMenuElements(ImGuiContext *pImGuiContext,
                                                  void *pUserData,
                                                  char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiDeveloperConsole *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiDeveloperConsole::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiDeveloperConsole *>(pUserData);

    pData->customUI();
}

bool foeImGuiDeveloperConsole::viewMainMenu() {
    if (ImGui::MenuItem("Developer Console")) {
        mOpen = true;
        mFocus = true;
    }

    return true;
}

void foeImGuiDeveloperConsole::customUI() {
    if (!mOpen)
        return;

    if (mFocus) {
        ImGui::SetNextWindowFocus();
        mFocus = false;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiWindowFlags_NoScrollbar);
    ImGui::Begin("DeveloperConsole", &mOpen);

    if (mBuffer.empty())
        mBuffer.resize(512);
    if (ImGui::InputText("", mBuffer.data(), mBuffer.size(),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        runCommand(mBuffer);
    }

    ImGui::BeginTable("DeveloperConsoleOutputTable", 3, ImGuiTableFlags_Resizable);

    ImGui::TableSetupColumn("Category");
    ImGui::TableSetupColumn("Level");
    ImGui::TableSetupColumn("Message");
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(mEntries.size());

    while (clipper.Step()) {
        for (int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; ++idx) {
            ImGui::TableNextRow();

            if (mEntries.size() - 1 - idx < 0)
                continue;

            auto &entry = mEntries[mEntries.size() - 1 - idx];

            // Category
            ImGui::TableNextColumn();

            std::string category{entry.pCategory->name()};
            ImGui::Text("%s", category.data());

            // Level
            ImGui::TableNextColumn();

            ImGui::Text("%s", std::to_string(entry.level).data());

            // Message
            ImGui::TableNextColumn();

            ImGui::Text("%s", entry.message.data());
        }
    }

    ImGui::EndTable();

    ImGui::End();
}