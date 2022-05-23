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

#include <foe/xr/openxr/core.hpp>

#include <foe/engine_detail.h>

#include "xr_result.h"

foeResult foeOpenXrEnumerateApiLayerProperties(std::vector<XrApiLayerProperties> &properties) {
    uint32_t propertyCount;
    XrResult xrResult = xrEnumerateApiLayerProperties(0, &propertyCount, nullptr);
    if (xrResult != XR_SUCCESS)
        return xr_to_foeResult(xrResult);

    properties.resize(propertyCount);
    for (auto &it : properties) {
        it.type = XR_TYPE_API_LAYER_PROPERTIES;
    }

    xrResult = xrEnumerateApiLayerProperties(static_cast<uint32_t>(properties.size()),
                                             &propertyCount, properties.data());

    return xr_to_foeResult(xrResult);
}

foeResult foeOpenXrEnumerateInstanceExtensionProperties(
    char const *pApiLayerName, std::vector<XrExtensionProperties> &properties) {
    uint32_t propertyCount;
    XrResult xrResult =
        xrEnumerateInstanceExtensionProperties(pApiLayerName, 0, &propertyCount, nullptr);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    properties.resize(propertyCount);
    for (auto &it : properties) {
        it.type = XR_TYPE_EXTENSION_PROPERTIES;
    }

    xrResult = xrEnumerateInstanceExtensionProperties(
        pApiLayerName, static_cast<uint32_t>(properties.size()), &propertyCount, properties.data());

    return xr_to_foeResult(xrResult);
}

foeResult foeOpenXrEnumerateReferenceSpaces(XrSession xrSession,
                                            std::vector<XrReferenceSpaceType> &spaces) {
    uint32_t spaceCount;
    XrResult xrResult = xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    spaces.resize(spaceCount);
    xrResult = xrEnumerateReferenceSpaces(xrSession, static_cast<uint32_t>(spaces.size()),
                                          &spaceCount, spaces.data());

    return xr_to_foeResult(xrResult);
}

foeResult foeOpenXrEnumerateSwapchainFormats(XrSession xrSession, std::vector<int64_t> &formats) {
    uint32_t formatCount;
    XrResult xrResult = xrEnumerateSwapchainFormats(xrSession, 0, &formatCount, nullptr);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    formats.resize(formatCount);
    xrResult = xrEnumerateSwapchainFormats(xrSession, static_cast<uint32_t>(formats.size()),
                                           &formatCount, formats.data());

    return xr_to_foeResult(xrResult);
}