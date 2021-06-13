/*
    Copyright (C) 2021 George Cave.

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

#ifndef XR_VK_SESSION_VIEW_HPP
#define XR_VK_SESSION_VIEW_HPP

#include <foe/xr/vulkan.hpp>
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