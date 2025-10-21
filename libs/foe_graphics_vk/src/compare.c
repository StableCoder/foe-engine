// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/compare.h>

#include <foe/external/vk_struct_compare.h>
#include <foe/graphics/vk/shader.h>

bool compare_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo const *pData1,
                                      foeGfxVkShaderCreateInfo const *pData2) {
    // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
    if (pData1->builtinSetLayouts != pData2->builtinSetLayouts) {
        return false;
    }

    // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
    if (!compare_VkDescriptorSetLayoutCreateInfo(&pData1->descriptorSetLayoutCI,
                                                 &pData2->descriptorSetLayoutCI)) {
        return false;
    }

    // VkPushConstantRange - pushConstantRange
    if (!compare_VkPushConstantRange(&pData1->pushConstantRange, &pData2->pushConstantRange)) {
        return false;
    }

    return true;
}
