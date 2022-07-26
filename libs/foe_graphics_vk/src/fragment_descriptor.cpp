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
    pRasterizationSCI{},
    pDepthStencilSCI{},
    pColourBlendAttachments{},
    pColourBlendSCI{} {
    if (pRasterizationSCI)
        this->pRasterizationSCI = new VkPipelineRasterizationStateCreateInfo{*pRasterizationSCI};

    if (pDepthStencilSCI)
        this->pDepthStencilSCI = new VkPipelineDepthStencilStateCreateInfo{*pDepthStencilSCI};

    if (pColourBlendSCI) {
        this->pColourBlendSCI = new VkPipelineColorBlendStateCreateInfo{*pColourBlendSCI};

        pColourBlendAttachments =
            new VkPipelineColorBlendAttachmentState[pColourBlendSCI->attachmentCount];

        std::copy(pColourBlendSCI->pAttachments,
                  pColourBlendSCI->pAttachments + pColourBlendSCI->attachmentCount,
                  pColourBlendAttachments);
        this->pColourBlendSCI->pAttachments = pColourBlendAttachments;
    }
}

foeGfxVkFragmentDescriptor::~foeGfxVkFragmentDescriptor() {
    if (pColourBlendAttachments)
        delete[] pColourBlendAttachments;
    if (pColourBlendSCI)
        delete pColourBlendSCI;

    if (pDepthStencilSCI)
        delete pDepthStencilSCI;

    if (pRasterizationSCI)
        delete pRasterizationSCI;
}

auto foeGfxVkFragmentDescriptor::getBuiltinSetLayouts() const noexcept
    -> foeBuiltinDescriptorSetLayoutFlags {
    foeBuiltinDescriptorSetLayoutFlags flags = 0;

    if (mFragment)
        flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mFragment);

    return flags;
}

auto foeGfxVkFragmentDescriptor::getColourBlendSCI() noexcept
    -> VkPipelineColorBlendStateCreateInfo const * {
    if (pColourBlendSCI)
        pColourBlendSCI->pAttachments = pColourBlendAttachments;

    return pColourBlendSCI;
}
