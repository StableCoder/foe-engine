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

#include <foe/graphics/shader.hpp>
#include <foe/graphics/vk/shader.hpp>

#include <vk_error_code.hpp>

#include "session.hpp"
#include "shader.hpp"

namespace {

void foeGfxVkDestroyShader(foeGfxVkSession *pSession, foeGfxVkShader *pShader) {
    if (pShader->module != VK_NULL_HANDLE)
        vkDestroyShaderModule(pSession->device, pShader->module, nullptr);

    delete pShader;
}

} // namespace

std::error_code foeGfxVkCreateShader(foeGfxSession session,
                                     foeGfxVkShaderCreateInfo const *pCreateInfo,
                                     uint32_t shaderCodeSize,
                                     uint32_t const *pShaderCode,
                                     foeGfxShader *pShader) {
    auto *pSession = session_from_handle(session);

    VkResult vkRes{VK_SUCCESS};
    auto *pNew = new foeGfxVkShader;

    // Get the DescriptorSetLayout directly from the gfxSession
    VkDescriptorSetLayout layout =
        pSession->descriptorSetLayoutPool.get(&pCreateInfo->descriptorSetLayoutCI);

    *pNew = foeGfxVkShader{
        .builtinSetLayouts = pCreateInfo->builtinSetLayouts,
        .descriptorSetLayout = layout,
        .pushConstantRange = pCreateInfo->pushConstantRange,
    };

    VkShaderModuleCreateInfo shaderCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderCodeSize,
        .pCode = pShaderCode,
    };

    vkRes = vkCreateShaderModule(pSession->device, &shaderCI, nullptr, &pNew->module);
    if (vkRes != VK_SUCCESS)
        goto CREATE_FAILED;

CREATE_FAILED:
    if (vkRes == VK_SUCCESS) {
        *pShader = shader_to_handle(pNew);
    } else {
        foeGfxVkDestroyShader(pSession, pNew);
    }

    return vkRes;
}

void foeGfxDestroyShader(foeGfxSession session, foeGfxShader shader) {
    auto *pSession = session_from_handle(session);
    auto *pShader = shader_from_handle(shader);

    foeGfxVkDestroyShader(pSession, pShader);
}

foeBuiltinDescriptorSetLayoutFlags foeGfxShaderGetBuiltinDescriptorSetLayouts(foeGfxShader shader) {
    auto *pShader = shader_from_handle(shader);

    return pShader->builtinSetLayouts;
}

auto foeGfxVkGetShaderDescriptorSetLayout(foeGfxShader shader) -> VkDescriptorSetLayout {
    auto *pShader = shader_from_handle(shader);

    return pShader->descriptorSetLayout;
}

auto foeGfxVkGetShaderPushConstantRange(foeGfxShader shader) -> VkPushConstantRange {
    auto *pShader = shader_from_handle(shader);

    return pShader->pushConstantRange;
}