// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/fragment_descriptor.h>

extern "C" foeBuiltinDescriptorSetLayoutFlags foeGfxVkGetFragmentDescriptorBuiltinSetLayouts(
    foeGfxVkFragmentDescriptor const *pFragmentDescriptor) {
    foeBuiltinDescriptorSetLayoutFlags flags = 0;

    if (pFragmentDescriptor->mFragment)
        flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(pFragmentDescriptor->mFragment);

    return flags;
}

extern "C" VkPipelineColorBlendStateCreateInfo const *foeGfxVkGetFragmentDescriptorColourBlendSCI(
    foeGfxVkFragmentDescriptor const *pFragmentDescriptor) {
    if (pFragmentDescriptor->pColourBlendSCI)
        pFragmentDescriptor->pColourBlendSCI->pAttachments =
            pFragmentDescriptor->pColourBlendAttachments;

    return pFragmentDescriptor->pColourBlendSCI;
}
