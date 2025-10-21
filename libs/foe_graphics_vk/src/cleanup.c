// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/cleanup.h>

#include <foe/external/vk_struct_cleanup.h>
#include <foe/graphics/vk/shader.h>

#include <stddef.h>
#include <stdlib.h>

void cleanup_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo *pData) {
    // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
    cleanup_VkDescriptorSetLayoutCreateInfo(
        (VkDescriptorSetLayoutCreateInfo *)&pData->descriptorSetLayoutCI);
}
