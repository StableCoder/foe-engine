// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "debug_utils.hpp"

#include "log.hpp"

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
                                 void * /*userData*/) {
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        FOE_LOG(foeOpenXr, Error, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        FOE_LOG(foeOpenXr, Warning, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        FOE_LOG(foeOpenXr, Info, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }
    if ((messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        FOE_LOG(foeOpenXr, Verbose, "[{}] : {}", to_string(messageTypes), callbackData->message)
    }

    return XR_FALSE;
}

} // namespace

XrResult foeOpenXrCreateDebugUtilsMessenger(XrInstance instance,
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

XrResult foeOpenXrDestroyDebugUtilsMessenger(XrInstance instance,
                                             XrDebugUtilsMessengerEXT debugMessenger) {
    PFN_xrDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger{nullptr};
    XrResult res = xrGetInstanceProcAddr(instance, "xrDestroyDebugUtilsMessengerEXT",
                                         (PFN_xrVoidFunction *)&DestroyDebugUtilsMessenger);
    if (res != XR_SUCCESS) {
        return res;
    }

    DestroyDebugUtilsMessenger(debugMessenger);

    return XR_SUCCESS;
}