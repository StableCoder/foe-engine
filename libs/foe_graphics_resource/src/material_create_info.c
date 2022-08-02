// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/material_create_info.h>

#include <vk_struct_cleanup.h>

#include <stdlib.h>

void foeCleanup_foeMaterialCreateInfo(foeMaterialCreateInfo *pCreateInfo) {
    if (pCreateInfo->pColourBlendSCI) {
        cleanup_VkPipelineColorBlendStateCreateInfo(pCreateInfo->pColourBlendSCI);
        free(pCreateInfo->pColourBlendSCI);
    }

    if (pCreateInfo->pDepthStencilSCI) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(pCreateInfo->pDepthStencilSCI);
        free(pCreateInfo->pDepthStencilSCI);
    }

    if (pCreateInfo->pRasterizationSCI) {
        cleanup_VkPipelineRasterizationStateCreateInfo(pCreateInfo->pRasterizationSCI);
        free(pCreateInfo->pRasterizationSCI);
    }
}