// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_SHADER_HPP
#define FOE_GRAPHICS_VK_SHADER_HPP

#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

struct foeGfxVkShader {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts = 0;
    VkShaderModule module{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPushConstantRange pushConstantRange{};
};

FOE_DEFINE_HANDLE_CASTS(shader, foeGfxVkShader, foeGfxShader)

#endif // FOE_GRAPHICS_VK_SHADER_HPP