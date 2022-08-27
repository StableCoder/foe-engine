// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/cleanup.h>

#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/vk/cleanup.h>
#include <vk_struct_cleanup.h>

#include <stddef.h>
#include <stdlib.h>

void cleanup_foeImageCreateInfo(foeImageCreateInfo *pData) {
    // char const * - pFile
    if (pData->pFile) {
        free((char *)pData->pFile);
    }
}

void cleanup_foeMeshFileCreateInfo(foeMeshFileCreateInfo *pData) {
    // char const * - pMesh
    if (pData->pMesh) {
        free((char *)pData->pMesh);
    }

    // char const * - pFile
    if (pData->pFile) {
        free((char *)pData->pFile);
    }
}

void cleanup_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo *pData) {}

void cleanup_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo *pData) {}

void cleanup_foeMaterialCreateInfo(foeMaterialCreateInfo *pData) {
    // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
    if (pData->pColourBlendSCI) {
        cleanup_VkPipelineColorBlendStateCreateInfo(
            (VkPipelineColorBlendStateCreateInfo *)pData->pColourBlendSCI);
        free((VkPipelineColorBlendStateCreateInfo *)pData->pColourBlendSCI);
    }

    // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
    if (pData->pDepthStencilSCI) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(
            (VkPipelineDepthStencilStateCreateInfo *)pData->pDepthStencilSCI);
        free((VkPipelineDepthStencilStateCreateInfo *)pData->pDepthStencilSCI);
    }

    // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
    if (pData->pRasterizationSCI) {
        cleanup_VkPipelineRasterizationStateCreateInfo(
            (VkPipelineRasterizationStateCreateInfo *)pData->pRasterizationSCI);
        free((VkPipelineRasterizationStateCreateInfo *)pData->pRasterizationSCI);
    }
}

void cleanup_foeShaderCreateInfo(foeShaderCreateInfo *pData) {
    // foeGfxVkShaderCreateInfo - gfxCreateInfo
    cleanup_foeGfxVkShaderCreateInfo((foeGfxVkShaderCreateInfo *)&pData->gfxCreateInfo);

    // char const * - pFile
    if (pData->pFile) {
        free((char *)pData->pFile);
    }
}

void cleanup_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo *pData) {
    // VkPipelineTessellationStateCreateInfo - tessellationSCI
    cleanup_VkPipelineTessellationStateCreateInfo(
        (VkPipelineTessellationStateCreateInfo *)&pData->tessellationSCI);

    // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
    cleanup_VkPipelineInputAssemblyStateCreateInfo(
        (VkPipelineInputAssemblyStateCreateInfo *)&pData->inputAssemblySCI);

    // VkVertexInputAttributeDescription* - pInputAttributes
    if (pData->pInputAttributes) {
        free((VkVertexInputAttributeDescription *)pData->pInputAttributes);
    }

    // VkVertexInputBindingDescription* - pInputBindings
    if (pData->pInputBindings) {
        free((VkVertexInputBindingDescription *)pData->pInputBindings);
    }

    // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
    cleanup_VkPipelineVertexInputStateCreateInfo(
        (VkPipelineVertexInputStateCreateInfo *)&pData->vertexInputSCI);
}
