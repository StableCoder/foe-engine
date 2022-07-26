// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>
#include <vulkan/vulkan.h>

#include <vector>

struct FOE_GFX_RES_EXPORT foeMaterialCreateInfo {
    ~foeMaterialCreateInfo();

    foeId fragmentShader = FOE_INVALID_ID;
    foeId image = FOE_INVALID_ID;
    VkPipelineRasterizationStateCreateInfo *pRasterizationSCI;
    VkPipelineDepthStencilStateCreateInfo *pDepthStencilSCI;
    VkPipelineColorBlendStateCreateInfo *pColourBlendSCI;
    std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
};

FOE_GFX_RES_EXPORT void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type,
                                                     void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP