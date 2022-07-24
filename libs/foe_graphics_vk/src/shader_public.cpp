// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/shader.h>
#include <foe/graphics/vk/shader.hpp>

#include <vk_struct_cleanup.h>

#include "session.hpp"
#include "shader.hpp"
#include "vk_result.h"

namespace {

void foeGfxVkDestroyShader(foeGfxVkSession *pSession, foeGfxVkShader *pShader) {
    if (pShader->module != VK_NULL_HANDLE)
        vkDestroyShaderModule(pSession->device, pShader->module, nullptr);

    delete pShader;
}

} // namespace

void foeGfxVkDestroyShaderCreateInfo(foeGfxVkShaderCreateInfo const *pCreateInfo) {
    cleanup_VkDescriptorSetLayoutCreateInfo(&pCreateInfo->descriptorSetLayoutCI);
}

foeResultSet foeGfxVkCreateShader(foeGfxSession session,
                                  foeGfxVkShaderCreateInfo const *pCreateInfo,
                                  uint32_t shaderCodeSize,
                                  uint32_t const *pShaderCode,
                                  foeGfxShader *pShader) {
    auto *pSession = session_from_handle(session);

    VkResult vkResult = VK_SUCCESS;
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

    vkResult = vkCreateShaderModule(pSession->device, &shaderCI, nullptr, &pNew->module);
    if (vkResult != VK_SUCCESS)
        goto CREATE_FAILED;

CREATE_FAILED:
    if (vkResult == VK_SUCCESS) {
        *pShader = shader_to_handle(pNew);
    } else {
        foeGfxVkDestroyShader(pSession, pNew);
    }

    return vk_to_foeResult(vkResult);
}

extern "C" void foeGfxDestroyShader(foeGfxSession session, foeGfxShader shader) {
    auto *pSession = session_from_handle(session);
    auto *pShader = shader_from_handle(shader);

    foeGfxVkDestroyShader(pSession, pShader);
}

extern "C" foeBuiltinDescriptorSetLayoutFlags foeGfxShaderGetBuiltinDescriptorSetLayouts(
    foeGfxShader shader) {
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