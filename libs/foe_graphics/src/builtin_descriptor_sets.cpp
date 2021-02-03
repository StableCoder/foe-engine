/*
    Copyright (C) 2020 George Cave.

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

#include <foe/graphics/builtin_descriptor_sets.hpp>

#include <limits>

std::string to_string(foeBuiltinDescriptorSetLayoutFlagBits builtinSetLayout) {
    switch (builtinSetLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return "ProjectionViewMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return "ModelMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return "BoneStateMatrices";

    default:
        return {};
    }
}

foeBuiltinDescriptorSetLayoutFlagBits to_builtin_set_layout(std::string_view str) {
    if (str == "ProjectionViewMatrix")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX;

    if (str == "ModelMatrix")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX;

    if (str == "BoneStateMatrices")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES;

    return static_cast<foeBuiltinDescriptorSetLayoutFlagBits>(0);
}
