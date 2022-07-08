// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/image_create_info.hpp>
#include <imgui.h>

void imgui_foeImage(foeImage const *pResource) { ImGui::Text("foeImage"); }

void imgui_foeImageCreateInfo(foeImageCreateInfo const *pCreateInfo) {
    ImGui::Text("foeImageCreateInfo");

    ImGui::Text("File: %s", pCreateInfo->fileName.c_str());
}