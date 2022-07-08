// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/vk/vulkan.hpp>

#include <memory>

#include "xr_result.h"

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
    xrRes = xrGetSystem(instance, &xrSystemGetInfo, &systemId);
    if (xrRes == XR_SUCCESS)
        systemIds.emplace_back(systemId);

    return systemIds;
}

} // namespace

foeResult foeXrGetVulkanInstanceExtensions(XrInstance instance,
                                           std::vector<std::string> &extensions) {
    PFN_xrGetVulkanInstanceExtensionsKHR GetVulkanInstanceExtensions{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanInstanceExtensions);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    // Iterate through all systems available annd add their required extensions
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferCapacity;
        xrResult = GetVulkanInstanceExtensions(instance, id, 0, &bufferCapacity, nullptr);
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        std::unique_ptr<char[]> buffer{new char[bufferCapacity]};
        xrResult = GetVulkanInstanceExtensions(instance, id, bufferCapacity, &bufferCapacity,
                                               buffer.get());
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        auto newExtensions = splitString(buffer.get(), bufferCapacity);
        extensions.insert(extensions.end(), newExtensions.begin(), newExtensions.end());
    }

    return xr_to_foeResult(xrResult);
}

foeResult foeXrGetVulkanDeviceExtensions(XrInstance instance,
                                         std::vector<std::string> &extensions) {
    PFN_xrGetVulkanDeviceExtensionsKHR GetVulkanDeviceExtensions{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanDeviceExtensions);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    // Iterate through all systems available annd add their required extensions
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferCapacity;
        xrResult = GetVulkanDeviceExtensions(instance, id, 0, &bufferCapacity, nullptr);
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        std::unique_ptr<char[]> buffer{new char[bufferCapacity]};
        xrResult =
            GetVulkanDeviceExtensions(instance, id, bufferCapacity, &bufferCapacity, buffer.get());
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        auto newExtensions = splitString(buffer.get(), bufferCapacity);
        extensions.insert(extensions.end(), newExtensions.begin(), newExtensions.end());
    }

    return xr_to_foeResult(xrResult);
}

foeResult foeXrGetVulkanGraphicsDevice(XrInstance instance,
                                       XrSystemId systemId,
                                       VkInstance vkInstance,
                                       VkPhysicalDevice *vkPhysicalDevice) {
    PFN_xrGetVulkanGraphicsDeviceKHR GetVulkanGraphicsDevice{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanGraphicsDevice);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    xrResult = GetVulkanGraphicsDevice(instance, systemId, vkInstance, vkPhysicalDevice);

    return xr_to_foeResult(xrResult);
}

foeResult foeXrGetVulkanGraphicsRequirements(
    XrInstance instance,
    XrSystemId systemId,
    XrGraphicsRequirementsVulkanKHR *graphicsRequirements) {
    PFN_xrGetVulkanGraphicsRequirementsKHR GetVulkanGraphicsRequirements{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanGraphicsRequirements);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    graphicsRequirements->type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

    xrResult = GetVulkanGraphicsRequirements(instance, systemId, graphicsRequirements);

    return xr_to_foeResult(xrResult);
}

foeResult foeOpenXrEnumerateSwapchainVkImages(XrSwapchain xrSwapchain,
                                              std::vector<XrSwapchainImageVulkanKHR> &images) {
    uint32_t imageCount;
    XrResult xrResult = xrEnumerateSwapchainImages(xrSwapchain, 0, &imageCount, nullptr);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    images.resize(imageCount);
    xrResult =
        xrEnumerateSwapchainImages(xrSwapchain, static_cast<uint32_t>(images.size()), &imageCount,
                                   reinterpret_cast<XrSwapchainImageBaseHeader *>(images.data()));

    return xr_to_foeResult(xrResult);
}