/*
    Copyright (C) 2020-2022 George Cave.

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

#include <foe/graphics/builtin_descriptor_sets.h>

#include <string.h>

char const *builtin_set_layout_to_string(foeBuiltinDescriptorSetLayoutFlagBits builtinSetLayout) {
    switch (builtinSetLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return "ProjectionViewMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return "ModelMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return "BoneStateMatrices";

    default:
        return NULL;
    }
}

foeBuiltinDescriptorSetLayoutFlagBits string_to_builtin_set_layout(char const *pString) {
    if (strcmp(pString, "ProjectionViewMatrix") == 0)
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX;

    if (strcmp(pString, "ModelMatrix") == 0)
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX;

    if (strcmp(pString, "BoneStateMatrices") == 0)
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES;

    return (foeBuiltinDescriptorSetLayoutFlagBits)0;
}
