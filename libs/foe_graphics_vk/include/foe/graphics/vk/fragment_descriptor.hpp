// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#include <memory>

struct foeGfxVkFragmentDescriptor {
    FOE_GFX_EXPORT foeGfxVkFragmentDescriptor(
        VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
        VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
        VkPipelineColorBlendStateCreateInfo const *pColourBlendAttachmentSCI,
        foeGfxShader fragment);

    FOE_GFX_EXPORT foeGfxVkFragmentDescriptor &operator=(foeGfxVkFragmentDescriptor &&) noexcept =
        default;

    FOE_GFX_EXPORT ~foeGfxVkFragmentDescriptor();

    FOE_GFX_EXPORT auto getBuiltinSetLayouts() const noexcept -> foeBuiltinDescriptorSetLayoutFlags;

    FOE_GFX_EXPORT auto getColourBlendSCI() noexcept -> VkPipelineColorBlendStateCreateInfo const *;

    foeGfxShader mFragment = nullptr;

    VkPipelineRasterizationStateCreateInfo *pRasterizationSCI = nullptr;
    VkPipelineDepthStencilStateCreateInfo *pDepthStencilSCI = nullptr;
    VkPipelineColorBlendStateCreateInfo *pColourBlendSCI = nullptr;
    VkPipelineColorBlendAttachmentState *pColourBlendAttachments = nullptr;
};

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_HPP