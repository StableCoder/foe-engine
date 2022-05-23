/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/wsi/vulkan.h>

#include "result.h"
#include "vk_result.h"
#include "window.hpp"

namespace {

foeResult foeWsiWindowGetVulkanExtensionsErrC(uint32_t *pExtensionCount,
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

foeResult foeWsiWindowGetVulkanExtensions(uint32_t *pExtensionCount, char const ***pppExtensions) {
    return foeWsiWindowGetVulkanExtensionsErrC(pExtensionCount, pppExtensions);
}

foeResult foeWsiWindowGetVkSurface(foeWsiWindow window,
                                   VkInstance instance,
                                   VkSurfaceKHR *pSurface) {
    auto *pWindow = window_from_handle(window);

    return vk_to_foeResult(glfwCreateWindowSurface(instance, pWindow->pWindow, nullptr, pSurface));
}