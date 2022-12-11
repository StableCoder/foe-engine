// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/fragment_descriptor_pool.h>

#include "fragment_descriptor_pool.hpp"

extern "C" foeGfxVkFragmentDescriptor *foeGfxVkGetFragmentDescriptor(
    foeGfxVkFragmentDescriptorPool fragmentDescriptorPool,
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
    foeGfxShader fragment) {
    foeGfxVkFragmentDescriptorPoolImpl *pFragmentDescriptorPool =
        fragment_descriptor_pool_from_handle(fragmentDescriptorPool);

    return pFragmentDescriptorPool->get(pRasterizationSCI, pDepthStencilSCI, pColourBlendSCI,
                                        fragment);
}