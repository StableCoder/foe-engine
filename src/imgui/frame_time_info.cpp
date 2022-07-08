// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "frame_time_info.hpp"

#include <foe/imgui/state.hpp>
#include <imgui.h>

#include "../frame_timer.hpp"

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

}

foeImGuiFrameTimeInfo::foeImGuiFrameTimeInfo(FrameTimer *pFrameTimer) : mFrameTimer{pFrameTimer} {}

bool foeImGuiFrameTimeInfo::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiFrameTimeInfo::renderMenuElements,
                         foeImGuiFrameTimeInfo::renderCustomUI, renderMenus.data(),
                         renderMenus.size());
}

void foeImGuiFrameTimeInfo::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiFrameTimeInfo::renderMenuElements,
                     foeImGuiFrameTimeInfo::renderCustomUI, renderMenus.data(), renderMenus.size());
}

bool foeImGuiFrameTimeInfo::renderMenuElements(ImGuiContext *pImGuiContext,
                                               void *pUserData,
                                               char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiFrameTimeInfo *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiFrameTimeInfo::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiFrameTimeInfo *>(pUserData);

    pData->customUI();
}

bool foeImGuiFrameTimeInfo::viewMainMenu() {
    if (ImGui::MenuItem("Frame Time Info")) {
        mOpen = !mOpen;
    }

    return true;
}

void foeImGuiFrameTimeInfo::customUI() {
    if (!mOpen) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiWindowFlags_NoScrollbar);
    ImGui::Begin("Frame Time Info", &mOpen);

    ImGui::Text("%.2f ms/frame (%.1f fps)",
                (double)mFrameTimer->averageFrameTime<std::chrono::nanoseconds>().count() /
                    1000000.,
                mFrameTimer->framesPerSecond());

    ImGui::End();
}