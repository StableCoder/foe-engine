// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
