// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_DEBUG_UTILS_HPP
#define FOE_XR_DEBUG_UTILS_HPP

#include <foe/xr/export.h>
#include <openxr/openxr.h>

FOE_XR_EXPORT
XrResult foeOpenXrCreateDebugUtilsMessenger(XrInstance instance,
                                            XrDebugUtilsMessengerEXT *pDebugMessenger);

FOE_XR_EXPORT
XrResult foeOpenXrDestroyDebugUtilsMessenger(XrInstance instance,
                                             XrDebugUtilsMessengerEXT debugMessenger);

#endif // FOE_XR_DEBUG_UTILS_HPP