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

#include "xr.hpp"

#include <foe/graphics/backend.h>
#include <foe/graphics/vk/session.hpp>
#include <foe/xr/error_code.hpp>
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/vulkan.hpp>

#include <cstring>

std::error_code createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime) {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (strcmp(foeGfxBackendName(), "Vulkan") == 0) {
        extensions.emplace_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
    } else {
        std::abort();
    }

    return foeXrOpenCreateRuntime("FoE Engine", 0, layers, extensions, false, debugLogging,
                                  pRuntime);
}

std::error_code createXrSession(foeXrRuntime runtime,
                                foeGfxSession gfxSession,
                                foeXrSession *pSession) {
    XrResult xrRes{XR_SUCCESS};
    XrSystemId xrSystemId{};

    // OpenXR SystemId
    if (foeXrOpenGetInstance(runtime) != XR_NULL_HANDLE) {
        XrSystemGetInfo systemGetInfo{
            .type = XR_TYPE_SYSTEM_GET_INFO,
            .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
        };

        xrRes = xrGetSystem(foeXrOpenGetInstance(runtime), &systemGetInfo, &xrSystemId);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }
    }

    // Types
    uint32_t viewConfigCount;
    xrRes = xrEnumerateViewConfigurations(foeXrOpenGetInstance(runtime), xrSystemId, 0,
                                          &viewConfigCount, nullptr);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    std::vector<XrViewConfigurationType> xrViewConfigTypes;
    xrViewConfigTypes.resize(viewConfigCount);

    xrRes = xrEnumerateViewConfigurations(foeXrOpenGetInstance(runtime), xrSystemId,
                                          xrViewConfigTypes.size(), &viewConfigCount,
                                          xrViewConfigTypes.data());
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    // Is View Mutable??
    XrViewConfigurationProperties xrViewConfigProps{.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
    xrRes = xrGetViewConfigurationProperties(foeXrOpenGetInstance(runtime), xrSystemId,
                                             xrViewConfigTypes[0], &xrViewConfigProps);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    // Check graphics requirements
    XrGraphicsRequirementsVulkanKHR gfxRequirements;
    xrRes = foeXrGetVulkanGraphicsRequirements(foeXrOpenGetInstance(runtime), xrSystemId,
                                               &gfxRequirements);

    // XrSession
    XrGraphicsBindingVulkanKHR gfxBinding{
        .type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
        .instance = foeGfxVkGetInstance(gfxSession),
        .physicalDevice = foeGfxVkGetPhysicalDevice(gfxSession),
        .device = foeGfxVkGetDevice(gfxSession),
        .queueFamilyIndex = 0,
        .queueIndex = 0,
    };
    return pSession->createSession(foeXrOpenGetInstance(runtime), xrSystemId, xrViewConfigTypes[0],
                                   &gfxBinding);
}