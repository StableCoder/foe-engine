// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>
#include <vulkan/vulkan.h>

#include <vector>

struct foeVertexDescriptorCreateInfo {
    foeId vertexShader;
    foeId tessellationControlShader;
    foeId tessellationEvaluationShader;
    foeId geometryShader;
    VkPipelineVertexInputStateCreateInfo vertexInputSCI;
    std::vector<VkVertexInputBindingDescription> inputBindings;
    std::vector<VkVertexInputAttributeDescription> inputAttributes;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblySCI;
    VkPipelineTessellationStateCreateInfo tessellationSCI;
};

FOE_GFX_RES_EXPORT void foeDestroyVertexDescriptorCreateInfo(foeResourceCreateInfoType type,
                                                             void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_CREATE_INFO_HPP