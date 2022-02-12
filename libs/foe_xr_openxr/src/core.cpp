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

#include <foe/xr/openxr/core.hpp>

#include <foe/engine_detail.h>
#include <foe/xr/openxr/error_code.hpp>

#include <cstring>

auto foeXrEnumerateApiLayerProperties(std::vector<XrApiLayerProperties> &properties)
    -> std::error_code {
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

auto foeXrEnumerateInstanceExtensionProperties(char const *pApiLayerName,
                                               std::vector<XrExtensionProperties> &properties)
    -> std::error_code {
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

auto foeXrEnumerateReferenceSpaces(XrSession xrSession, std::vector<XrReferenceSpaceType> &spaces)
    -> std::error_code {
    uint32_t spaceCount;
    XrResult res = xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    spaces.resize(spaceCount);
    return xrEnumerateReferenceSpaces(xrSession, static_cast<uint32_t>(spaces.size()), &spaceCount,
                                      spaces.data());
}

auto foeXrEnumerateSwapchainFormats(XrSession xrSession, std::vector<int64_t> &formats)
    -> std::error_code {
    uint32_t formatCount;
    XrResult res = xrEnumerateSwapchainFormats(xrSession, 0, &formatCount, nullptr);
    if (res != XR_SUCCESS) {
        return res;
    }

    formats.resize(formatCount);
    return xrEnumerateSwapchainFormats(xrSession, static_cast<uint32_t>(formats.size()),
                                       &formatCount, formats.data());
}