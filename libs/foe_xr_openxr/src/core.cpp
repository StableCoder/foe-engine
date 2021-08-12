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

#include <foe/xr/core.hpp>

#include <foe/engine_detail.h>

#include <cstring>

XrResult foeXrEnumerateApiLayerProperties(std::vector<XrApiLayerProperties> &properties) {
    uint32_t propertyCount;
    XrResult res = xrEnumerateApiLayerProperties(0, &propertyCount, nullptr);
    if (res != XR_SUCCESS)
        return res;

    properties.resize(propertyCount);
    for (auto &it : properties) {
        it.type = XR_TYPE_API_LAYER_PROPERTIES;
    }

    return xrEnumerateApiLayerProperties(static_cast<uint32_t>(properties.size()), &propertyCount,
                                         properties.data());
}

XrResult foeXrEnumerateInstanceExtensionProperties(char const *pApiLayerName,
                                                   std::vector<XrExtensionProperties> &properties) {
    uint32_t propertyCount;
    XrResult res =
        xrEnumerateInstanceExtensionProperties(pApiLayerName, 0, &propertyCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    properties.resize(propertyCount);
    for (auto &it : properties) {
        it.type = XR_TYPE_EXTENSION_PROPERTIES;
    }

    return xrEnumerateInstanceExtensionProperties(
        pApiLayerName, static_cast<uint32_t>(properties.size()), &propertyCount, properties.data());
}

XrResult foeXrCreateInstance(char const *appName,
                             uint32_t appVersion,
                             std::vector<std::string> apiLayers,
                             std::vector<std::string> extensions,
                             XrInstance *pInstance) {
    XrApplicationInfo appInfo{
        .applicationVersion = appVersion,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = XR_MAKE_VERSION(1, 0, 12),
    };
    strncpy(appInfo.applicationName, appName, XR_MAX_APPLICATION_NAME_SIZE);
    strncpy(appInfo.engineName, FOE_ENGINE_NAME, XR_MAX_ENGINE_NAME_SIZE);

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    for (auto &it : apiLayers) {
        finalLayers.emplace_back(it.data());
    }
    for (auto &it : extensions) {
        finalExtensions.emplace_back(it.data());
    }

    XrInstanceCreateInfo instanceCI = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = appInfo,
        .enabledApiLayerCount = static_cast<uint32_t>(apiLayers.size()),
        .enabledApiLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .enabledExtensionNames = finalExtensions.data(),
    };

    return xrCreateInstance(&instanceCI, pInstance);
}

XrResult foeXrEnumerateReferenceSpaces(XrSession xrSession,
                                       std::vector<XrReferenceSpaceType> &spaces) {
    uint32_t spaceCount;
    XrResult res = xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    spaces.resize(spaceCount);
    return xrEnumerateReferenceSpaces(xrSession, static_cast<uint32_t>(spaces.size()), &spaceCount,
                                      spaces.data());
}

XrResult foeXrEnumerateSwapchainFormats(XrSession xrSession, std::vector<int64_t> &formats) {
    uint32_t formatCount;
    XrResult res = xrEnumerateSwapchainFormats(xrSession, 0, &formatCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    formats.resize(formatCount);
    return xrEnumerateSwapchainFormats(xrSession, static_cast<uint32_t>(formats.size()),
                                       &formatCount, formats.data());
}