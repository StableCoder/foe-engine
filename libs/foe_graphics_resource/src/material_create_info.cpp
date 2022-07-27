// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/material_create_info.hpp>

#include <vk_struct_cleanup.h>

#include <stdlib.h>

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMaterialCreateInfo *)pCreateInfo;

    if (pCI->pColourBlendSCI) {
        cleanup_VkPipelineColorBlendStateCreateInfo(pCI->pColourBlendSCI);
        free(pCI->pColourBlendSCI);
    }
    if (pCI->pDepthStencilSCI != nullptr) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(pCI->pDepthStencilSCI);
        free(pCI->pDepthStencilSCI);
    }
    if (pCI->pRasterizationSCI != nullptr) {
        cleanup_VkPipelineRasterizationStateCreateInfo(pCI->pRasterizationSCI);
        free(pCI->pRasterizationSCI);
    }
}