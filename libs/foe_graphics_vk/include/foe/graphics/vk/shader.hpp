/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_GRPAHICS_VK_SHADER_HPP
#define FOE_GRPAHICS_VK_SHADER_HPP

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

struct foeGfxVkShaderCreateInfo {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts;
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI;
    VkPushConstantRange pushConstantRange;
};

FOE_GFX_EXPORT void foeGfxVkDestroyShaderCreateInfo(foeGfxVkShaderCreateInfo const *pCreateInfo);

FOE_GFX_EXPORT foeResult foeGfxVkCreateShader(foeGfxSession session,
                                              foeGfxVkShaderCreateInfo const *pCreateInfo,
                                              uint32_t shaderCodeSize,
                                              uint32_t const *pShaderCode,
                                              foeGfxShader *pShader);

FOE_GFX_EXPORT auto foeGfxVkGetShaderDescriptorSetLayout(foeGfxShader shader)
    -> VkDescriptorSetLayout;

FOE_GFX_EXPORT auto foeGfxVkGetShaderPushConstantRange(foeGfxShader shader) -> VkPushConstantRange;

#endif // FOE_GRPAHICS_VK_SHADER_HPP