// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/fragment_descriptor.hpp>

#include <algorithm>

foeGfxVkFragmentDescriptor::foeGfxVkFragmentDescriptor(
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
    foeGfxShader fragment) :
    mFragment{fragment},
    hasRasterizationSCI{pRasterizationSCI != nullptr},
    mRasterizationSCI{},
    hasDepthStencilSCI{pDepthStencilSCI != nullptr},
    mDepthStencilSCI{},
    hasColourBlendSCI{pColourBlendSCI != nullptr},
    mColourBlendAttachments{},
    mColourBlendSCI{} {
    if (hasRasterizationSCI)
        mRasterizationSCI = *pRasterizationSCI;

    if (hasDepthStencilSCI)
        mDepthStencilSCI = *pDepthStencilSCI;

    if (hasColourBlendSCI) {
        mColourBlendSCI = *pColourBlendSCI;

        mColourBlendAttachments.reset(
            new VkPipelineColorBlendAttachmentState[pColourBlendSCI->attachmentCount]);

        std::copy(pColourBlendSCI->pAttachments,
                  pColourBlendSCI->pAttachments + pColourBlendSCI->attachmentCount,
                  mColourBlendAttachments.get());
        mColourBlendSCI.pAttachments = mColourBlendAttachments.get();
    }
}

foeGfxVkFragmentDescriptor::~foeGfxVkFragmentDescriptor() {}

auto foeGfxVkFragmentDescriptor::getBuiltinSetLayouts() const noexcept
    -> foeBuiltinDescriptorSetLayoutFlags {
    foeBuiltinDescriptorSetLayoutFlags flags = 0;

    if (mFragment != nullptr)
        flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mFragment);

    return flags;
}

auto foeGfxVkFragmentDescriptor::getColourBlendSCI() noexcept
    -> VkPipelineColorBlendStateCreateInfo const * {
    mColourBlendSCI.pAttachments = mColourBlendAttachments.get();
    return hasColourBlendSCI ? &mColourBlendSCI : nullptr;
}
