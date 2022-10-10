// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/vk/vulkan.h>

#include <vector>

#include "xr_result.h"

namespace {

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

extern "C" foeResultSet foeXrGetVulkanInstanceExtensions(XrInstance instance,
                                                         uint32_t *pExtensionsLength,
                                                         char *pExtensions) {
    PFN_xrGetVulkanInstanceExtensionsKHR GetVulkanInstanceExtensions{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanInstanceExtensions);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    // Iterate through all systems available annd add their required extensions
    uint32_t totalBufferCapacity = 0;
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferSize;
        xrResult = GetVulkanInstanceExtensions(instance, id, 0, &bufferSize, nullptr);
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        if (pExtensions == nullptr) {
            totalBufferCapacity += bufferSize;
            continue;
        }

        if (bufferSize < (*pExtensionsLength - totalBufferCapacity))
            bufferSize = (*pExtensionsLength - totalBufferCapacity);

        xrResult = GetVulkanInstanceExtensions(instance, id, bufferSize, &bufferSize, pExtensions);
        totalBufferCapacity += bufferSize;
        if (xrResult != XR_SUCCESS) {
            break;
        }

        pExtensions += bufferSize;
    }

    *pExtensionsLength = totalBufferCapacity;

    return xr_to_foeResult(xrResult);
}

extern "C" foeResultSet foeXrGetVulkanDeviceExtensions(XrInstance instance,
                                                       uint32_t *pExtensionsLength,
                                                       char *pExtensions) {
    PFN_xrGetVulkanDeviceExtensionsKHR GetVulkanDeviceExtensions{nullptr};
    XrResult xrResult = xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR",
                                              (PFN_xrVoidFunction *)&GetVulkanDeviceExtensions);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    // Iterate through all systems available annd add their required extensions
    uint32_t totalBufferCapacity = 0;
    auto systemIds = getAllSystemIds(instance);
    for (auto id : systemIds) {
        uint32_t bufferSize;
        xrResult = GetVulkanDeviceExtensions(instance, id, 0, &bufferSize, nullptr);
        if (xrResult != XR_SUCCESS) {
            return xr_to_foeResult(xrResult);
        }

        if (pExtensions == nullptr) {
            totalBufferCapacity += bufferSize;
            continue;
        }

        if (bufferSize < (*pExtensionsLength - totalBufferCapacity))
            bufferSize = (*pExtensionsLength - totalBufferCapacity);

        xrResult = GetVulkanDeviceExtensions(instance, id, bufferSize, &bufferSize, pExtensions);
        totalBufferCapacity += bufferSize;
        if (xrResult != XR_SUCCESS) {
            break;
        }

        pExtensions += bufferSize;
    }

    *pExtensionsLength = totalBufferCapacity;

    return xr_to_foeResult(xrResult);
}

extern "C" foeResultSet foeXrGetVulkanGraphicsDevice(XrInstance instance,
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

extern "C" foeResultSet foeXrGetVulkanGraphicsRequirements(
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

extern "C" foeResultSet foeOpenXrEnumerateSwapchainVkImages(XrSwapchain xrSwapchain,
                                                            uint32_t *pImageCount,
                                                            XrSwapchainImageVulkanKHR *pImages) {
    XrResult xrResult;
    if (pImages == NULL)
        xrResult = xrEnumerateSwapchainImages(xrSwapchain, 0, pImageCount, NULL);
    else
        xrResult = xrEnumerateSwapchainImages(xrSwapchain, *pImageCount, pImageCount,
                                              (XrSwapchainImageBaseHeader *)pImages);
    return xr_to_foeResult(xrResult);
}