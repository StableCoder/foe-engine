// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "xr.hpp"

#include <foe/graphics/backend.h>
#include <foe/graphics/vk/session.hpp>
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/openxr/vk/vulkan.hpp>

#include "xr_result.h"

#include <cstring>

foeResultSet createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime) {
    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    if (strcmp(foeGfxBackendName(), "Vulkan") == 0) {
        extensions.emplace_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
    } else {
        std::abort();
    }

    return foeOpenXrCreateRuntime("FoE Engine", 0, layers.size(), layers.data(), extensions.size(),
                                  extensions.data(), false, debugLogging, pRuntime);
}

foeResultSet createXrSession(foeXrRuntime runtime,
                             foeGfxSession gfxSession,
                             foeOpenXrSession *pSession) {
    foeResultSet result;
    XrSystemId xrSystemId{};

    // OpenXR SystemId
    if (foeOpenXrGetInstance(runtime) != XR_NULL_HANDLE) {
        XrSystemGetInfo systemGetInfo{
            .type = XR_TYPE_SYSTEM_GET_INFO,
            .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
        };

        result = xr_to_foeResult(
            xrGetSystem(foeOpenXrGetInstance(runtime), &systemGetInfo, &xrSystemId));
        if (result.value != FOE_SUCCESS) {
            return result;
        }
    }

    // Types
    uint32_t viewConfigCount;
    result = xr_to_foeResult(xrEnumerateViewConfigurations(
        foeOpenXrGetInstance(runtime), xrSystemId, 0, &viewConfigCount, nullptr));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    std::vector<XrViewConfigurationType> xrViewConfigTypes;
    xrViewConfigTypes.resize(viewConfigCount);

    result = xr_to_foeResult(xrEnumerateViewConfigurations(
        foeOpenXrGetInstance(runtime), xrSystemId, xrViewConfigTypes.size(), &viewConfigCount,
        xrViewConfigTypes.data()));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // Is View Mutable??
    XrViewConfigurationProperties xrViewConfigProps{.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
    result = xr_to_foeResult(xrGetViewConfigurationProperties(
        foeOpenXrGetInstance(runtime), xrSystemId, xrViewConfigTypes[0], &xrViewConfigProps));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // Check graphics requirements
    XrGraphicsRequirementsVulkanKHR gfxRequirements;
    result = foeXrGetVulkanGraphicsRequirements(foeOpenXrGetInstance(runtime), xrSystemId,
                                                &gfxRequirements);
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // XrSession
    XrGraphicsBindingVulkanKHR gfxBinding{
        .type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
        .instance = foeGfxVkGetInstance(gfxSession),
        .physicalDevice = foeGfxVkGetPhysicalDevice(gfxSession),
        .device = foeGfxVkGetDevice(gfxSession),
        .queueFamilyIndex = 0,
        .queueIndex = 0,
    };

    return pSession->createSession(runtime, xrSystemId, xrViewConfigTypes[0], &gfxBinding);
}