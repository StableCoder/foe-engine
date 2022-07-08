// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_IMAGE_HPP
#define FOE_GRAPHICS_RESOURCE_IMAGE_HPP

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct foeImage {
    VmaAllocation alloc;
    VkImage image;
    VkImageView view;
    VkSampler sampler;
};

#endif // FOE_GRAPHICS_RESOURCE_IMAGE_HPP