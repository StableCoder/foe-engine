/*
    Copyright (C) 2021-2022 George Cave.

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

#include "xr.hpp"

#include <foe/graphics/backend.h>
#include <foe/graphics/vk/session.hpp>
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/openxr/vk/vulkan.hpp>

#include "xr_result.h"

#include <cstring>

foeResult createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime) {
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

foeResult createXrSession(foeXrRuntime runtime,
                          foeGfxSession gfxSession,
                          foeOpenXrSession *pSession) {
    foeResult result;
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