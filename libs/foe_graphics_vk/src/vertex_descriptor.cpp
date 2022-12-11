// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/vertex_descriptor.h>

#include <stdlib.h>

extern "C" foeBuiltinDescriptorSetLayoutFlags foeGfxVkGetVertexDescriptorBuiltinSetLayouts(
    foeGfxVkVertexDescriptor const *pVertexDescriptor) {
    foeBuiltinDescriptorSetLayoutFlags flags = 0;

    if (pVertexDescriptor->mVertex != nullptr)
        flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(pVertexDescriptor->mVertex);

    if (pVertexDescriptor->mTessellationControl != nullptr)
        flags |=
            foeGfxShaderGetBuiltinDescriptorSetLayouts(pVertexDescriptor->mTessellationControl);

    if (pVertexDescriptor->mTessellationEvaluation != nullptr)
        flags |=
            foeGfxShaderGetBuiltinDescriptorSetLayouts(pVertexDescriptor->mTessellationEvaluation);

    if (pVertexDescriptor->mGeometry != nullptr)
        flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(pVertexDescriptor->mGeometry);

    return flags;
}

extern "C" VkPipelineVertexInputStateCreateInfo const *foeGfxVkGetVertexDescriptorVertexInputSCI(
    foeGfxVkVertexDescriptor const *pVertexDescriptor) {

    const_cast<foeGfxVkVertexDescriptor *>(pVertexDescriptor)->mVertexInputSCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = pVertexDescriptor->vertexInputBindingCount,
        .pVertexBindingDescriptions = pVertexDescriptor->pVertexInputBindings,
        .vertexAttributeDescriptionCount = pVertexDescriptor->vertexInputAttributeCount,
        .pVertexAttributeDescriptions = pVertexDescriptor->pVertexInputAttributes,
    };

    return &pVertexDescriptor->mVertexInputSCI;
}

extern "C" VkPipelineTessellationStateCreateInfo const *foeGfxVkGetVertexDescriptorTessellationSCI(
    foeGfxVkVertexDescriptor const *pVertexDescriptor) {
    if (pVertexDescriptor->mTessellationControl != nullptr ||
        pVertexDescriptor->mTessellationEvaluation != nullptr) {
        return &pVertexDescriptor->mTessellationSCI;
    }

    return nullptr;
}

extern "C" void cleanup_foeGfxVkVertexDescriptor(foeGfxVkVertexDescriptor *pData) {
    if (pData->pVertexInputAttributes)
        free(pData->pVertexInputAttributes);
    if (pData->pVertexInputBindings)
        free(pData->pVertexInputBindings);
}