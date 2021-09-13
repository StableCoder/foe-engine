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

#include "render_state.hpp"

#include <imgui.h>

#include "../render_state.hpp"

void imgui_foeRenderState(foeRenderState *pComponent) {
    ImGui::Separator();
    ImGui::Text("foeRenderState");

    ImGui::Text("Vertex Descriptor: %08x", pComponent->vertexDescriptor);

    ImGui::Text("Boned Vertex Descriptor: %p",
                reinterpret_cast<void *>(pComponent->boneDescriptorSet));

    ImGui::Text("Material: %08x", pComponent->material);

    ImGui::Text("Mesh: %08x", pComponent->mesh);
}