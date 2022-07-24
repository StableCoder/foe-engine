// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/wsi/vulkan.h>

#include "result.h"
#include "vk_result.h"
#include "window.hpp"

namespace {

foeResultSet foeWsiWindowGetVulkanExtensionsErrC(uint32_t *pExtensionCount,
                                                 char const ***pppExtensions) {
    if (!glfwInit()) {
        return to_foeResult(FOE_WSI_ERROR_FAILED_TO_INITIALIZE_BACKEND);
    }

    if (!glfwVulkanSupported()) {
        return to_foeResult(FOE_WSI_ERROR_VULKAN_NOT_SUPPORTED);
    }

    *pppExtensions = glfwGetRequiredInstanceExtensions(pExtensionCount);

    return to_foeResult(FOE_WSI_SUCCESS);
}

} // namespace

foeResultSet foeWsiWindowGetVulkanExtensions(uint32_t *pExtensionCount,
                                             char const ***pppExtensions) {
    return foeWsiWindowGetVulkanExtensionsErrC(pExtensionCount, pppExtensions);
}

foeResultSet foeWsiWindowGetVkSurface(foeWsiWindow window,
                                      VkInstance instance,
                                      VkSurfaceKHR *pSurface) {
    auto *pWindow = window_from_handle(window);

    return vk_to_foeResult(glfwCreateWindowSurface(instance, pWindow->pWindow, nullptr, pSurface));
}