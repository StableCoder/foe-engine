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

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP

#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct foeFragmentDescriptor {
    auto getBuiltinSetLayouts() const noexcept -> foeBuiltinDescriptorSetLayoutFlags {
        foeBuiltinDescriptorSetLayoutFlags flags = 0;

        if (mFragment != nullptr)
            flags |= mFragment->builtinSetLayouts;

        return flags;
    }

    auto getColourBlendSCI() noexcept -> VkPipelineColorBlendStateCreateInfo const * {
        mColourBlendSCI.attachmentCount = mColourBlendAttachments.size();
        mColourBlendSCI.pAttachments = mColourBlendAttachments.data();

        return &mColourBlendSCI;
    }

    foeShader *mFragment = nullptr;

    VkPipelineRasterizationStateCreateInfo mRasterizationSCI{};
    VkPipelineDepthStencilStateCreateInfo mDepthStencilSCI{};

    std::vector<VkPipelineColorBlendAttachmentState> mColourBlendAttachments;
    VkPipelineColorBlendStateCreateInfo mColourBlendSCI{};
};

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP