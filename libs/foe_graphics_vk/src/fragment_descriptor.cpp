/*
    Copyright (C) 2020-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
