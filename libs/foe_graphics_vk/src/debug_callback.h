// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DEBUG_CALLBACK_H
#define DEBUG_CALLBACK_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

VkResult foeVkCreateDebugCallback(VkInstance instance, VkDebugReportCallbackEXT *pDebugCallback);

VkResult foeVkDestroyDebugCallback(VkInstance instance, VkDebugReportCallbackEXT debugCallback);

#ifdef __cplusplus
}
#endif

#endif // DEBUG_CALLBACK_H