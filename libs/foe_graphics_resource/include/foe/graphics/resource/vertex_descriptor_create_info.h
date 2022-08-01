// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_H
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_H

#include <foe/ecs/id.h>
#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeVertexDescriptorCreateInfo {
    foeId vertexShader;
    foeId tessellationControlShader;
    foeId tessellationEvaluationShader;
    foeId geometryShader;
    VkPipelineVertexInputStateCreateInfo vertexInputSCI;
    uint32_t inputBindingCount;
    VkVertexInputBindingDescription *pInputBindings;
    uint32_t inputAttributeCount;
    VkVertexInputAttributeDescription *pInputAttributes;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblySCI;
    VkPipelineTessellationStateCreateInfo tessellationSCI;
} foeVertexDescriptorCreateInfo;

FOE_GFX_RES_EXPORT void foeDestroyVertexDescriptorCreateInfo(foeResourceCreateInfoType type,
                                                             void *pCreateInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_H