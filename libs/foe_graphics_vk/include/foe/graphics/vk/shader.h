// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRPAHICS_VK_SHADER_H
#define FOE_GRPAHICS_VK_SHADER_H

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkShaderCreateInfo {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts;
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI;
    VkPushConstantRange pushConstantRange;
} foeGfxVkShaderCreateInfo;

FOE_GFX_EXPORT void foeClean_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo const *pCreateInfo);

FOE_GFX_EXPORT foeResultSet foeGfxVkCreateShader(foeGfxSession session,
                                                 foeGfxVkShaderCreateInfo const *pCreateInfo,
                                                 uint32_t shaderCodeSize,
                                                 uint32_t const *pShaderCode,
                                                 foeGfxShader *pShader);

FOE_GFX_EXPORT VkDescriptorSetLayout foeGfxVkGetShaderDescriptorSetLayout(foeGfxShader shader);

FOE_GFX_EXPORT VkPushConstantRange foeGfxVkGetShaderPushConstantRange(foeGfxShader shader);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRPAHICS_VK_SHADER_H