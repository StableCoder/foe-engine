// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VERTEX_DESCRIPTOR_H
#define FOE_GRAPHICS_VERTEX_DESCRIPTOR_H

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkVertexDescriptor {
    foeGfxShader mVertex;
    foeGfxShader mTessellationControl;
    foeGfxShader mTessellationEvaluation;
    foeGfxShader mGeometry;

    uint32_t vertexInputBindingCount;
    VkVertexInputBindingDescription *pVertexInputBindings;
    uint32_t vertexInputAttributeCount;
    VkVertexInputAttributeDescription *pVertexInputAttributes;
    VkPipelineVertexInputStateCreateInfo mVertexInputSCI;

    VkPipelineInputAssemblyStateCreateInfo mInputAssemblySCI;

    VkPipelineTessellationStateCreateInfo mTessellationSCI;
} foeGfxVkVertexDescriptor;

FOE_GFX_EXPORT foeBuiltinDescriptorSetLayoutFlags
foeGfxVkGetVertexDescriptorBuiltinSetLayouts(foeGfxVkVertexDescriptor const *pVertexDescriptor);

FOE_GFX_EXPORT VkPipelineVertexInputStateCreateInfo const *
foeGfxVkGetVertexDescriptorVertexInputSCI(foeGfxVkVertexDescriptor const *pVertexDescriptor);

FOE_GFX_EXPORT VkPipelineTessellationStateCreateInfo const *
foeGfxVkGetVertexDescriptorTessellationSCI(foeGfxVkVertexDescriptor const *pVertexDescriptor);

FOE_GFX_EXPORT void cleanup_foeGfxVkVertexDescriptor(foeGfxVkVertexDescriptor *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VERTEX_DESCRIPTOR_H