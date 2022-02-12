/*
    Copyright (C) 2020-2022 George Cave.

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

#ifndef FOE_XR_OPENXR_VK_VULKAN_HPP
#define FOE_XR_OPENXR_VK_VULKAN_HPP

#include <foe/xr/openxr/vk/export.h>
#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <string>
#include <system_error>
#include <vector>

FOE_OPENXR_VK_EXPORT auto foeXrGetVulkanInstanceExtensions(XrInstance instance,
                                                           std::vector<std::string> &extensions)
    -> std::error_code;

FOE_OPENXR_VK_EXPORT auto foeXrGetVulkanDeviceExtensions(XrInstance instance,
                                                         std::vector<std::string> &extensions)
    -> std::error_code;

FOE_OPENXR_VK_EXPORT auto foeXrGetVulkanGraphicsDevice(XrInstance instance,
                                                       XrSystemId systemId,
                                                       VkInstance vkInstance,
                                                       VkPhysicalDevice *vkPhysicalDevice)
    -> std::error_code;

FOE_OPENXR_VK_EXPORT auto foeXrGetVulkanGraphicsRequirements(
    XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsVulkanKHR *graphicsRequirements)
    -> std::error_code;

FOE_OPENXR_VK_EXPORT auto foeXrEnumerateSwapchainVkImages(
    XrSwapchain swapchain, std::vector<XrSwapchainImageVulkanKHR> &images) -> std::error_code;

#endif // FOE_XR_OPENXR_VK_VULKAN_HPP