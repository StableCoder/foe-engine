// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/imgui/create_info.h>

#include <imgui.h>

extern "C" void imgui_foeResourceCreateInfo(foeResourceCreateInfo createInfo) {
    ImGui::Text("Type: %i", foeResourceCreateInfoGetType(createInfo));
    // Decrement ref-count by 1 to account for handle being held for display purposes
    ImGui::Text("Ref Count: %i", foeResourceCreateInfoGetRefCount(createInfo) - 1);
}