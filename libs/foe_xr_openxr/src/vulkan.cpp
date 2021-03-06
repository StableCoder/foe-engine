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

#include <foe/xr/vulkan.hpp>

#include <memory>

namespace {

std::vector<std::string> splitString(char const *buffer, uint32_t length) {
    std::vector<std::string> tokens;

    char const *beg = buffer;
    char const *end = buffer + 1;
    while (*end != '\0' && end != beg + length) {
        if (*end == ' ') {
            tokens.emplace_back(std::string{beg, end});
            beg = end;
            ++beg;
        }
        ++end;
    }

    if (beg != end) {
        tokens.emplace_back(std::string{beg, end});
    }

    return tokens;
}

std::vector<XrSystemId> getAllSystemIds(XrInstance instance) {
    std::vector<XrSystemId> systemIds;

    XrSystemId systemId;
    XrSystemGetInfo xrSystemGetInfo{
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
    };

    // SystemId - XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
    XrResult xrRes = xrGetSystem(instance, &xrSystemGetInfo, &systemId);
    if (xrRes == XR_SUCCESS)
        systemIds.emplace_back(systemId);

    // SystemId - XR_FORM_FACTOR_HANDHELD_DISPLAY
    xrSystemGetInfo.formFactor = XR_FORM_FACTOR_HANDHELD_DISPLAY;
    if (xrRes == XR_SUCCESS)
        systemIds.emplace_back(systemId);

    return systemIds;
}

} // namespace

XrResult foeXrGetVulkanInstanceExtensions(XrInstance instance,
                                          std::vector<std::string> &extensions) {
    PFN_xrGetVulkanInstanceExtensionsKHR GetVulkanInstanceExtensions{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR",
                                         (PFN_xrVoidFunction *)&GetVulkanInstanceExtensions);
    if (res != XR_SUCCESS) {
        return res;
    }

    // Iterate through all systems available annd add their required extensions
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferCapacity;
        res = GetVulkanInstanceExtensions(instance, id, 0, &bufferCapacity, nullptr);
        if (res != XR_SUCCESS) {
            return res;
        }

        std::unique_ptr<char[]> buffer{new char[bufferCapacity]};
        res = GetVulkanInstanceExtensions(instance, id, bufferCapacity, &bufferCapacity,
                                          buffer.get());
        if (res != XR_SUCCESS) {
            return res;
        }

        auto newExtensions = splitString(buffer.get(), bufferCapacity);
        extensions.insert(extensions.end(), newExtensions.begin(), newExtensions.end());
    }

    return res;
}

XrResult foeXrGetVulkanDeviceExtensions(XrInstance instance, std::vector<std::string> &extensions) {
    PFN_xrGetVulkanDeviceExtensionsKHR GetVulkanDeviceExtensions{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR",
                                         (PFN_xrVoidFunction *)&GetVulkanDeviceExtensions);
    if (res != XR_SUCCESS) {
        return res;
    }

    // Iterate through all systems available annd add their required extensions
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferCapacity;
        res = GetVulkanDeviceExtensions(instance, id, 0, &bufferCapacity, nullptr);
        if (res != XR_SUCCESS) {
            return res;
        }

        std::unique_ptr<char[]> buffer{new char[bufferCapacity]};
        res =
            GetVulkanDeviceExtensions(instance, id, bufferCapacity, &bufferCapacity, buffer.get());
        if (res != XR_SUCCESS) {
            return res;
        }

        auto newExtensions = splitString(buffer.get(), bufferCapacity);
        extensions.insert(extensions.end(), newExtensions.begin(), newExtensions.end());
    }

    return res;
}

XrResult foeXrGetVulkanGraphicsDevice(XrInstance instance,
                                      XrSystemId systemId,
                                      VkInstance vkInstance,
                                      VkPhysicalDevice *vkPhysicalDevice) {
    PFN_xrGetVulkanGraphicsDeviceKHR GetVulkanGraphicsDevice{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR",
                                         (PFN_xrVoidFunction *)&GetVulkanGraphicsDevice);
    if (res != XR_SUCCESS) {
        return res;
    }

    return GetVulkanGraphicsDevice(instance, systemId, vkInstance, vkPhysicalDevice);
}

XrResult foeXrGetVulkanGraphicsRequirements(XrInstance instance,
                                            XrSystemId systemId,
                                            XrGraphicsRequirementsVulkanKHR *graphicsRequirements) {
    PFN_xrGetVulkanGraphicsRequirementsKHR GetVulkanGraphicsRequirements{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR",
                                         (PFN_xrVoidFunction *)&GetVulkanGraphicsRequirements);
    if (res != XR_SUCCESS) {
        return res;
    }

    graphicsRequirements->type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

    return GetVulkanGraphicsRequirements(instance, systemId, graphicsRequirements);
}

XrResult foeXrEnumerateSwapchainVkImages(XrSwapchain xrSwapchain,
                                         std::vector<XrSwapchainImageVulkanKHR> &images) {
    uint32_t imageCount;
    XrResult res = xrEnumerateSwapchainImages(xrSwapchain, 0, &imageCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    images.resize(imageCount);
    return xrEnumerateSwapchainImages(
        xrSwapchain, images.size(), &imageCount,
        reinterpret_cast<XrSwapchainImageBaseHeader *>(images.data()));
}