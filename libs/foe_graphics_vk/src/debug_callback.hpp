// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DEBUG_CALLBACK_HPP
#define DEBUG_CALLBACK_HPP

#include <vulkan/vulkan.h>

VkResult foeVkCreateDebugCallback(VkInstance instance, VkDebugReportCallbackEXT *pDebugCallback);

VkResult foeVkDestroyDebugCallback(VkInstance instance, VkDebugReportCallbackEXT debugCallback);

#endif // DEBUG_CALLBACK_HPP