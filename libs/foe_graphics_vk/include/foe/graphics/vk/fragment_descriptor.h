// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_H
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_H

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkFragmentDescriptor {
    foeGfxShader mFragment;

    VkPipelineRasterizationStateCreateInfo *pRasterizationSCI;
    VkPipelineDepthStencilStateCreateInfo *pDepthStencilSCI;
    VkPipelineColorBlendStateCreateInfo *pColourBlendSCI;
    VkPipelineColorBlendAttachmentState *pColourBlendAttachments;
} foeGfxVkFragmentDescriptor;

FOE_GFX_EXPORT foeBuiltinDescriptorSetLayoutFlags foeGfxVkGetFragmentDescriptorBuiltinSetLayouts(
    foeGfxVkFragmentDescriptor const *pFragmentDescriptor);

FOE_GFX_EXPORT VkPipelineColorBlendStateCreateInfo const *
foeGfxVkGetFragmentDescriptorColourBlendSCI(foeGfxVkFragmentDescriptor const *pFragmentDescriptor);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_H