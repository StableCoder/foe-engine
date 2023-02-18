// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_state.hpp"

#include <imgui.h>

#include "../simulation/render_state.h"

void imgui_foeRenderState(foeRenderState *pComponent) {
    ImGui::Separator();
    ImGui::Text("foeRenderState");

    ImGui::Text("Vertex Descriptor: %08x", pComponent->vertexDescriptor);

    ImGui::Text("Boned Vertex Descriptor: %08x", pComponent->bonedVertexDescriptor);

    ImGui::Text("Material: %08x", pComponent->material);

    ImGui::Text("Mesh: %08x", pComponent->mesh);
}