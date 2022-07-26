// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/material_create_info.hpp>

#include <vk_struct_cleanup.h>

foeMaterialCreateInfo::~foeMaterialCreateInfo() {
    if (pColourBlendSCI != nullptr) {
        cleanup_VkPipelineColorBlendStateCreateInfo(pColourBlendSCI);
        delete pColourBlendSCI;
    }
    if (pDepthStencilSCI != nullptr) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(pDepthStencilSCI);
        delete pDepthStencilSCI;
    }
    if (pRasterizationSCI != nullptr) {
        cleanup_VkPipelineRasterizationStateCreateInfo(pRasterizationSCI);
        delete pRasterizationSCI;
    }
}

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMaterialCreateInfo *)pCreateInfo;
    pCI->~foeMaterialCreateInfo();
}