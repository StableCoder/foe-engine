// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_H
#define FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_H

#include <foe/ecs/id.h>
#include <foe/resource/create_info.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeMaterialCreateInfo {
    foeResourceID fragmentShader;
    foeResourceID image;
    VkPipelineRasterizationStateCreateInfo *pRasterizationSCI;
    VkPipelineDepthStencilStateCreateInfo *pDepthStencilSCI;
    VkPipelineColorBlendStateCreateInfo *pColourBlendSCI;
} foeMaterialCreateInfo;

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_H