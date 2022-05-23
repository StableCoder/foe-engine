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

#include <foe/graphics/imgui/builtin_descriptor_sets.hpp>

#include <imgui.h>

void imgui_foeBuiltinDescriptorSetLayoutFlags(std::string const &label,
                                              foeBuiltinDescriptorSetLayoutFlags const &data) {
    std::string serializedStr =
        builtin_set_layout_to_string((foeBuiltinDescriptorSetLayoutFlagBits)data);
    ImGui::Text("%s: %s", label.c_str(), serializedStr.c_str());
}