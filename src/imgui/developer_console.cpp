// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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

void foeImGuiDeveloperConsole::log(void *pContext,
                                   char const *pCategoryName,
                                   foeLogLevel level,
                                   char const *pMessage) {
    foeImGuiDeveloperConsole *pDevConsole = (foeImGuiDeveloperConsole *)pContext;

    std::scoped_lock lock{pDevConsole->mSync};

    pDevConsole->mEntries.emplace_back(Entry{
        .category = pCategoryName,
        .level = level,
        .message = pMessage,
    });

    if (pDevConsole->mEntries.size() > pDevConsole->mMaxEntries) {
        pDevConsole->mEntries.pop_front();
    }
}

size_t foeImGuiDeveloperConsole::maxEntries() const noexcept { return mMaxEntries; }

void foeImGuiDeveloperConsole::maxEntries(size_t numEntries) noexcept {
    std::scoped_lock lock{mSync};

    mMaxEntries = numEntries;
    while (mEntries.size() > mMaxEntries) {
        mEntries.pop_front();
    }
}

void foeImGuiDeveloperConsole::registerWithLogger() {
    foeLogRegisterSink(this, foeImGuiDeveloperConsole::log, nullptr);
}

void foeImGuiDeveloperConsole::deregisterFromLogger() {
    foeLogDeregisterSink(this, foeImGuiDeveloperConsole::log, nullptr);
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
        std::scoped_lock lock{mSync};
        mEntries.emplace_back(Entry{
            .category = "ConsoleInput",
            .level = FOE_LOG_LEVEL_INFO,
            .message = mBuffer,
        });
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

            ImGui::Text("%s", entry.category.c_str());

            // Level
            ImGui::TableNextColumn();

            ImGui::Text("%s", foeLogLevel_to_string(entry.level));

            // Message
            ImGui::TableNextColumn();

            ImGui::Text("%s", entry.message.data());
        }
    }

    ImGui::EndTable();

    ImGui::End();
}