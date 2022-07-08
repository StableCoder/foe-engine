// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/material_create_info.hpp>

#include <vk_struct_cleanup.h>

foeMaterialCreateInfo::~foeMaterialCreateInfo() {
    if (hasColourBlendSCI)
        cleanup_VkPipelineColorBlendStateCreateInfo(&colourBlendSCI);
    if (hasDepthStencilSCI)
        cleanup_VkPipelineDepthStencilStateCreateInfo(&depthStencilSCI);
    if (hasRasterizationSCI)
        cleanup_VkPipelineRasterizationStateCreateInfo(&rasterizationSCI);
}

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMaterialCreateInfo *)pCreateInfo;
    pCI->~foeMaterialCreateInfo();
}