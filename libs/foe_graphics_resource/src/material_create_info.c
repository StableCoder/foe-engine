// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/material_create_info.h>

#include <vk_struct_cleanup.h>

#include <stdlib.h>

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    foeMaterialCreateInfo *pCI = (foeMaterialCreateInfo *)pCreateInfo;

    if (pCI->pColourBlendSCI) {
        cleanup_VkPipelineColorBlendStateCreateInfo(pCI->pColourBlendSCI);
        free(pCI->pColourBlendSCI);
    }
    if (pCI->pDepthStencilSCI) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(pCI->pDepthStencilSCI);
        free(pCI->pDepthStencilSCI);
    }
    if (pCI->pRasterizationSCI) {
        cleanup_VkPipelineRasterizationStateCreateInfo(pCI->pRasterizationSCI);
        free(pCI->pRasterizationSCI);
    }
}