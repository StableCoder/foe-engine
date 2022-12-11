// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_PIPELINE_POOL_H
#define FOE_GRAPHICS_VK_PIPELINE_POOL_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVertexDescriptor foeGfxVertexDescriptor;
typedef struct foeGfxVkFragmentDescriptor foeGfxVkFragmentDescriptor;

FOE_DEFINE_HANDLE(foeGfxVkPipelinePool)

FOE_GFX_EXPORT foeGfxVkPipelinePool foeGfxVkGetPipelinePool(foeGfxSession session);

FOE_GFX_EXPORT foeResultSet foeGfxVkCreatePipelinePool(foeGfxSession session,
                                                       foeGfxVkPipelinePool *pPipelinePool);

FOE_GFX_EXPORT void foeGfxVkDestroyPipelinePool(foeGfxVkPipelinePool pipelinePool);

FOE_GFX_EXPORT foeResultSet foeGfxVkGetPipeline(foeGfxVkPipelinePool pipelinePool,
                                                foeGfxVertexDescriptor *vertexDescriptor,
                                                foeGfxVkFragmentDescriptor *fragmentDescriptor,
                                                VkRenderPass renderPass,
                                                uint32_t subpass,
                                                VkSampleCountFlags samples,
                                                VkPipelineLayout *pPipelineLayout,
                                                uint32_t *pDescriptorSetLayoutCount,
                                                VkPipeline *pPipeline);
#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_PIPELINE_POOL_H