// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/imgui/create_info.h>

#include <imgui.h>

extern "C" void imgui_foeResourceCreateInfo(foeResourceCreateInfo createInfo) {
    ImGui::Text("Type: %i", foeResourceCreateInfoGetType(createInfo));
    ImGui::Text("Ref Count: %i", foeResourceCreateInfoGetRefCount(createInfo));
}