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

#include <foe/graphics/export.h>
#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

#include <memory>

struct foeFragmentDescriptor {
    FOE_GFX_EXPORT foeFragmentDescriptor(
        VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
        VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
        VkPipelineColorBlendStateCreateInfo const *pColourBlendAttachmentSCI,
        foeGfxShader fragment);

    FOE_GFX_EXPORT foeFragmentDescriptor &operator=(foeFragmentDescriptor &&) noexcept = default;

    FOE_GFX_EXPORT ~foeFragmentDescriptor();

    FOE_GFX_EXPORT auto getBuiltinSetLayouts() const noexcept -> foeBuiltinDescriptorSetLayoutFlags;

    FOE_GFX_EXPORT auto getColourBlendSCI() noexcept -> VkPipelineColorBlendStateCreateInfo const *;

    foeGfxShader mFragment = nullptr;

    bool hasRasterizationSCI;
    VkPipelineRasterizationStateCreateInfo mRasterizationSCI{};

    bool hasDepthStencilSCI;
    VkPipelineDepthStencilStateCreateInfo mDepthStencilSCI{};

    bool hasColourBlendSCI;
    std::unique_ptr<VkPipelineColorBlendAttachmentState[]> mColourBlendAttachments;
    VkPipelineColorBlendStateCreateInfo mColourBlendSCI{};
};

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP