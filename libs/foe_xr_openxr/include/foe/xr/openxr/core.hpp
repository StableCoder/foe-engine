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

#ifndef FOE_XR_OPENXR_CORE_HPP
#define FOE_XR_OPENXR_CORE_HPP

#include <foe/xr/export.h>
#include <openxr/openxr.h>

#include <string>
#include <system_error>
#include <vector>

FOE_XR_EXPORT auto foeOpenXrEnumerateApiLayerProperties(
    std::vector<XrApiLayerProperties> &properties) -> std::error_code;

FOE_XR_EXPORT auto foeOpenXrEnumerateInstanceExtensionProperties(
    char const *pApiLayerName, std::vector<XrExtensionProperties> &properties) -> std::error_code;

FOE_XR_EXPORT auto foeOpenXrEnumerateReferenceSpaces(XrSession xrSession,
                                                     std::vector<XrReferenceSpaceType> &spaces)
    -> std::error_code;

FOE_XR_EXPORT auto foeOpenXrEnumerateSwapchainFormats(XrSession xrSession,
                                                      std::vector<int64_t> &formats)
    -> std::error_code;

#endif // FOE_XR_OPENXR_CORE_HPP