// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_CORE_HPP
#define FOE_XR_OPENXR_CORE_HPP

#include <foe/result.h>
#include <foe/xr/export.h>
#include <openxr/openxr.h>

#include <string>
#include <vector>

FOE_XR_EXPORT foeResultSet
foeOpenXrEnumerateApiLayerProperties(std::vector<XrApiLayerProperties> &properties);

FOE_XR_EXPORT foeResultSet foeOpenXrEnumerateInstanceExtensionProperties(
    char const *pApiLayerName, std::vector<XrExtensionProperties> &properties);

FOE_XR_EXPORT foeResultSet
foeOpenXrEnumerateReferenceSpaces(XrSession xrSession, std::vector<XrReferenceSpaceType> &spaces);

FOE_XR_EXPORT foeResultSet foeOpenXrEnumerateSwapchainFormats(XrSession xrSession,
                                                              std::vector<int64_t> &formats);

#endif // FOE_XR_OPENXR_CORE_HPP