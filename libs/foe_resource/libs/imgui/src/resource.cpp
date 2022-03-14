/*
    Copyright (C) 2022 George Cave.

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

#include <foe/resource/imgui/resource.h>

#include <imgui.h>

extern "C" void imgui_foeResource(foeResource resource) {
    ImGui::Text("Type: %i", foeResourceGetType(resource));

    // Is Loading
    if (foeResourceGetIsLoading(resource))
        ImGui::Text("Is Loading: true");
    else
        ImGui::Text("Is Loading: false");

    // State
    auto const state = foeResourceGetState(resource);
    if (state == foeResourceLoadState::Unloaded)
        ImGui::Text("LoadState: unloaded");
    else if (state == foeResourceLoadState::Loaded)
        ImGui::Text("LoadState: loaded");
    else if (state == foeResourceLoadState::Failed)
        ImGui::Text("LoadState: failed");

    // Counts
    ImGui::Text("Ref Count: %u", foeResourceGetRefCount(resource));
    ImGui::Text("Use Count: %u", foeResourceGetUseCount(resource));
}