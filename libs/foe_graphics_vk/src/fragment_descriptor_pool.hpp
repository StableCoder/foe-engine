// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FRAGMENT_DESCRIPTOR_POOL_HPP
#define FRAGMENT_DESCRIPTOR_POOL_HPP

#include <foe/graphics/vk/fragment_descriptor.h>
#include <foe/graphics/vk/fragment_descriptor_pool.h>
#include <vulkan/vulkan.h>

#include <mutex>
#include <vector>

class foeGfxVkFragmentDescriptorPoolImpl {
  public:
    ~foeGfxVkFragmentDescriptorPoolImpl();

    auto get(VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
             VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
             VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
             foeGfxShader fragment) -> foeGfxVkFragmentDescriptor *;

  private:
    std::mutex mSync;
    std::vector<foeGfxVkFragmentDescriptor *> mDescriptors;
};

FOE_DEFINE_HANDLE_CASTS(fragment_descriptor_pool,
                        foeGfxVkFragmentDescriptorPoolImpl,
                        foeGfxVkFragmentDescriptorPool)

#endif // FRAGMENT_DESCRIPTOR_POOL_HPP