// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VK_RESULT_H
#define VK_RESULT_H

#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void VkResultToString(VkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResultSet vk_to_foeResult(VkResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)VkResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // VK_RESULT_H