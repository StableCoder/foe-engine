/*
    Copyright (C) 2020 George Cave.

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

#include <foe/graphics/fragment_descriptor.hpp>

foeFragmentDescriptor::foeFragmentDescriptor(
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendAttachmentSCI,
    foeShader *pFragment) :
    mFragment{pFragment},
    hasRasterizationSCI{pRasterizationSCI != nullptr},
    mRasterizationSCI{*pRasterizationSCI},
    hasDepthStencilSCI{pDepthStencilSCI != nullptr},
    mDepthStencilSCI{*pDepthStencilSCI},
    mColourBlendAttachments{
        new VkPipelineColorBlendAttachmentState[pColourBlendAttachmentSCI->attachmentCount]},
    mColourBlendSCI{*pColourBlendAttachmentSCI} {
    std::copy(pColourBlendAttachmentSCI->pAttachments,
              pColourBlendAttachmentSCI->pAttachments + pColourBlendAttachmentSCI->attachmentCount,
              mColourBlendAttachments.get());
    mColourBlendSCI.pAttachments = mColourBlendAttachments.get();
}

foeFragmentDescriptor::~foeFragmentDescriptor() {}

auto foeFragmentDescriptor::getBuiltinSetLayouts() const noexcept
    -> foeBuiltinDescriptorSetLayoutFlags {
    foeBuiltinDescriptorSetLayoutFlags flags = 0;

    if (mFragment != nullptr)
        flags |= mFragment->builtinSetLayouts;

    return flags;
}

auto foeFragmentDescriptor::getColourBlendSCI() noexcept
    -> VkPipelineColorBlendStateCreateInfo const * {
    return &mColourBlendSCI;
}
