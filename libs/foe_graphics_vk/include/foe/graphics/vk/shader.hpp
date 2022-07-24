// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRPAHICS_VK_SHADER_HPP
#define FOE_GRPAHICS_VK_SHADER_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

struct foeGfxVkShaderCreateInfo {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts;
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI;
    VkPushConstantRange pushConstantRange;
};

FOE_GFX_EXPORT void foeGfxVkDestroyShaderCreateInfo(foeGfxVkShaderCreateInfo const *pCreateInfo);

FOE_GFX_EXPORT foeResultSet foeGfxVkCreateShader(foeGfxSession session,
                                                 foeGfxVkShaderCreateInfo const *pCreateInfo,
                                                 uint32_t shaderCodeSize,
                                                 uint32_t const *pShaderCode,
                                                 foeGfxShader *pShader);

FOE_GFX_EXPORT auto foeGfxVkGetShaderDescriptorSetLayout(foeGfxShader shader)
    -> VkDescriptorSetLayout;

FOE_GFX_EXPORT auto foeGfxVkGetShaderPushConstantRange(foeGfxShader shader) -> VkPushConstantRange;

#endif // FOE_GRPAHICS_VK_SHADER_HPP