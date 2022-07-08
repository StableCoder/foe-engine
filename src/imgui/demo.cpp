// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "demo.hpp"

#include <foe/imgui/state.hpp>
#include <imgui.h>

bool foeImGuiDemo::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, nullptr, foeImGuiDemo::renderCustomUI, nullptr, 0);
}

void foeImGuiDemo::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, nullptr, foeImGuiDemo::renderCustomUI, nullptr, 0);
}

void foeImGuiDemo::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::ShowDemoWindow();
}