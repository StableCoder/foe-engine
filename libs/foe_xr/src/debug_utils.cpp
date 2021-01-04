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

#include <foe/xr/debug_utils.hpp>

#include "xr_log.hpp"

namespace {

std::string to_string(XrDebugUtilsMessageTypeFlagsEXT types) {
    std::string str;

    if ((types & XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0) {
        str = "General";
    }
    if ((types & XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
        if (!str.empty()) {
            str += " | ";
        }
        str += "Validation";
    }
    if ((types & XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
        if (!str.empty()) {
            str += " | ";
        }
        str += "Performance";
    }
    if ((types & XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT) != 0) {
        if (!str.empty()) {
            str += " | ";
        }
        str += "Conformance";
    }

    return str;
}

XrBool32 openxrMessengerCallback(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                                 XrDebugUtilsMessageTypeFlagsEXT messageTypes,
                                 const XrDebugUtilsMessengerCallbackDataEXT *callbackData,
                                 void *userData) {
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        FOE_LOG(Xr, Error, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        FOE_LOG(Xr, Warning, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        FOE_LOG(Xr, Info, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        FOE_LOG(Xr, Verbose, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }

    return XR_FALSE;
}

} // namespace

XrResult foeXrCreateDebugUtilsMessenger(XrInstance instance,
                                        XrDebugUtilsMessengerEXT *pDebugMessenger) {
    if (instance == XR_NULL_HANDLE) {
        return XR_ERROR_HANDLE_INVALID;
    }

    PFN_xrCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrCreateDebugUtilsMessengerEXT",
                                         (PFN_xrVoidFunction *)&CreateDebugUtilsMessenger);
    if (res != XR_SUCCESS) {
        return res;
    }

    XrDebugUtilsMessengerCreateInfoEXT createInfo{
        .type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                             XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                             XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT,
        .userCallback = openxrMessengerCallback,
    };

    return CreateDebugUtilsMessenger(instance, &createInfo, pDebugMessenger);
}

XrResult foeXrDestroyDebugUtilsMessenger(XrInstance instance,
                                         XrDebugUtilsMessengerEXT debugMessenger) {
    PFN_xrDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrCreateDebugUtilsMessengerEXT",
                                         (PFN_xrVoidFunction *)&DestroyDebugUtilsMessenger);
    if (res != XR_SUCCESS) {
        return res;
    }

    DestroyDebugUtilsMessenger(debugMessenger);

    return XR_SUCCESS;
}