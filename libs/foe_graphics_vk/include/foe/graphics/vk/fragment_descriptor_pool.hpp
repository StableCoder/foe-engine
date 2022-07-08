// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#include <mutex>
#include <vector>

struct foeGfxVkFragmentDescriptor;

class foeGfxVkFragmentDescriptorPool {
  public:
    FOE_GFX_EXPORT ~foeGfxVkFragmentDescriptorPool();

    FOE_GFX_EXPORT auto get(VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
                            VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
                            VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
                            foeGfxShader fragment) -> foeGfxVkFragmentDescriptor *;

  private:
    std::mutex mSync;
    std::vector<foeGfxVkFragmentDescriptor *> mDescriptors;
};

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP