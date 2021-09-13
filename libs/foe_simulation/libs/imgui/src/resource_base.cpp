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

#include <foe/simulation/imgui/resource_base.hpp>

#include <foe/simulation/core/resource.hpp>
#include <imgui.h>

void imgui_renderResourceBase(const foeResourceBase *pResource) {
    // Is Loading
    if (pResource->getIsLoading())
        ImGui::Text("Is Loading: true");
    else
        ImGui::Text("Is Loading: false");

    // State
    auto const state = pResource->getState();
    if (state == foeResourceState::Unloaded)
        ImGui::Text("LoadState: unloaded");
    else if (state == foeResourceState::Loaded)
        ImGui::Text("LoadState: loaded");
    else if (state == foeResourceState::Failed)
        ImGui::Text("LoadState: failed");

    // Counts
    ImGui::Text("Ref Count: %u", pResource->getRefCount());
    ImGui::Text("Use Count: %u", pResource->getUseCount());
}