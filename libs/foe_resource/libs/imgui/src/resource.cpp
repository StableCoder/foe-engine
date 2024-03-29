// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/imgui/resource.h>

#include <imgui.h>

#include <limits>
#include <string>

extern "C" void imgui_foeResource(foeResource resource) {
    ImGui::Text("Type: %i", foeResourceGetType(resource));

    // State
    std::string stateStr = "";
    foeResourceStateFlags state = foeResourceGetState(resource);
    for (int i = 0; i < std::numeric_limits<foeResourceStateFlags>::digits; ++i) {
        uint32_t bit = (1 << i);
        if (state & bit) {
            if (!stateStr.empty())
                stateStr += " | ";
            stateStr += foeResourceStateFlagBitToString((foeResourceStateFlagBits)bit);
        }
    }
    ImGui::Text("State: %s", stateStr.c_str());

    // Counts
    // Reduce ref-count by 1 to account for handle being used for display purposes
    ImGui::Text("Ref Count: %u", foeResourceGetRefCount(resource) - 1);
    ImGui::Text("Use Count: %u", foeResourceGetUseCount(resource));
}