// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_VK_SESSION_VIEW_HPP
#define XR_VK_SESSION_VIEW_HPP

#include <foe/xr/openxr/vk/vulkan.hpp>
#include <vulkan/vulkan.h>

#include "xr_camera.hpp"

#include <vector>

struct foeXrVkSessionView {
    XrViewConfigurationView viewConfig;
    XrSwapchain swapchain;
    VkFormat format;
    std::vector<XrSwapchainImageVulkanKHR> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
    foeXrCamera camera;
};

#endif // XR_VK_SESSION_VIEW_HPP