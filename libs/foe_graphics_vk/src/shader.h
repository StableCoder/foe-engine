// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_H
#define SHADER_H

#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkShader {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts;
    VkShaderModule module;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPushConstantRange pushConstantRange;
} foeGfxVkShader;

FOE_DEFINE_HANDLE_CASTS(shader, foeGfxVkShader, foeGfxShader)

#ifdef __cplusplus
}
#endif

#endif // SHADER_H