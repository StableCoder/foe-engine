// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_H
#define FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_H

#include <foe/graphics/export.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBuiltinDescriptorSetLayoutFlagBits {
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX = 0x00000001,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX = 0x00000002,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES = 0x00000004,
} foeBuiltinDescriptorSetLayoutFlagBits;
typedef uint32_t foeBuiltinDescriptorSetLayoutFlags;

typedef enum foeDescriptorSetLayoutIndex {
    ProjectionViewMatrix = 0,
    ModelMatrix = 1,
    ModelAndBoneStateMatrices = 2,
    VertexShader = 3,
    TessellationControlShader = 4,
    TessellationEvaluationShader = 5,
    GeometryShader = 6,
    FragmentShader = 7,
} foeDescriptorSetLayoutIndex;

FOE_GFX_EXPORT
char const *builtin_set_layout_to_string(foeBuiltinDescriptorSetLayoutFlagBits builtinSetLayout);

FOE_GFX_EXPORT
foeBuiltinDescriptorSetLayoutFlagBits string_to_builtin_set_layout(char const *pString);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_H