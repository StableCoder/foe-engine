/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_GRAPHICS_VK_SHADER_HPP
#define FOE_GRAPHICS_VK_SHADER_HPP

#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

struct foeGfxVkShader {
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts = 0;
    VkShaderModule module{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPushConstantRange pushConstantRange{};
};

FOE_DEFINE_HANDLE_CASTS(shader, foeGfxVkShader, foeGfxShader)

#endif // FOE_GRAPHICS_VK_SHADER_HPP