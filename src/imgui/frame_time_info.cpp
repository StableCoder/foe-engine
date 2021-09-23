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

bool foeImGuiFrameTimeInfo::renderMenuElements(void *pContext, char const *pMenu) {
    auto *pData = static_cast<foeImGuiFrameTimeInfo *>(pContext);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiFrameTimeInfo::renderCustomUI(void *pContext) {
    auto *pData = static_cast<foeImGuiFrameTimeInfo *>(pContext);

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