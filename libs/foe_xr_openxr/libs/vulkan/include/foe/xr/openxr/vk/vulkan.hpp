// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_VK_VULKAN_HPP
#define FOE_XR_OPENXR_VK_VULKAN_HPP

#include <foe/result.h>
#include <foe/xr/openxr/vk/export.h>
#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>

FOE_OPENXR_VK_EXPORT foeResultSet
foeXrGetVulkanInstanceExtensions(XrInstance instance, std::vector<std::string> &extensions);

FOE_OPENXR_VK_EXPORT foeResultSet
foeXrGetVulkanDeviceExtensions(XrInstance instance, std::vector<std::string> &extensions);

FOE_OPENXR_VK_EXPORT foeResultSet foeXrGetVulkanGraphicsDevice(XrInstance instance,
                                                               XrSystemId systemId,
                                                               VkInstance vkInstance,
                                                               VkPhysicalDevice *vkPhysicalDevice);

FOE_OPENXR_VK_EXPORT foeResultSet
foeXrGetVulkanGraphicsRequirements(XrInstance instance,
                                   XrSystemId systemId,
                                   XrGraphicsRequirementsVulkanKHR *graphicsRequirements);

FOE_OPENXR_VK_EXPORT foeResultSet foeOpenXrEnumerateSwapchainVkImages(
    XrSwapchain swapchain, std::vector<XrSwapchainImageVulkanKHR> &images);

#endif // FOE_XR_OPENXR_VK_VULKAN_HPP