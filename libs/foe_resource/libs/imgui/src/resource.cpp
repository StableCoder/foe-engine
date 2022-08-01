// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
    if (state == FOE_RESOURCE_LOAD_STATE_UNLOADED)
        ImGui::Text("LoadState: unloaded");
    else if (state == FOE_RESOURCE_LOAD_STATE_LOADED)
        ImGui::Text("LoadState: loaded");
    else if (state == FOE_RESOURCE_LOAD_STATE_FAILED)
        ImGui::Text("LoadState: failed");

    // Counts
    ImGui::Text("Ref Count: %u", foeResourceGetRefCount(resource));
    ImGui::Text("Use Count: %u", foeResourceGetUseCount(resource));
}