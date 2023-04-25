// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_H
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_H

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <foe/handle.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeGfxVkFragmentDescriptor foeGfxVkFragmentDescriptor;

FOE_DEFINE_HANDLE(foeGfxVkFragmentDescriptorPool)

FOE_GFX_EXPORT
foeGfxVkFragmentDescriptor *foeGfxVkGetFragmentDescriptor(
    foeGfxVkFragmentDescriptorPool fragmentDescriptorPool,
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
    foeGfxShader fragment);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_H