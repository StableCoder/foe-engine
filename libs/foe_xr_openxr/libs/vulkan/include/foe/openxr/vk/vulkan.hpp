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

#ifndef FOE_OPENXR_VK_VULKAN_HPP
#define FOE_OPENXR_VK_VULKAN_HPP

#include <foe/openxr/vk/export.h>
#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>

FOE_OPENXR_VK_EXPORT XrResult
foeXrGetVulkanInstanceExtensions(XrInstance instance, std::vector<std::string> &extensions);

FOE_OPENXR_VK_EXPORT XrResult foeXrGetVulkanDeviceExtensions(XrInstance instance,
                                                             std::vector<std::string> &extensions);

FOE_OPENXR_VK_EXPORT XrResult foeXrGetVulkanGraphicsDevice(XrInstance instance,
                                                           XrSystemId systemId,
                                                           VkInstance vkInstance,
                                                           VkPhysicalDevice *vkPhysicalDevice);

FOE_OPENXR_VK_EXPORT XrResult
foeXrGetVulkanGraphicsRequirements(XrInstance instance,
                                   XrSystemId systemId,
                                   XrGraphicsRequirementsVulkanKHR *graphicsRequirements);

FOE_OPENXR_VK_EXPORT XrResult foeXrEnumerateSwapchainVkImages(
    XrSwapchain swapchain, std::vector<XrSwapchainImageVulkanKHR> &images);

#endif // FOE_OPENXR_VK_VULKAN_HPP