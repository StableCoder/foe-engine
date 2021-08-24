/*
    Copyright (C) 2020-2021 George Cave.

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

#ifndef FOE_WSI_VULKAN_HPP
#define FOE_WSI_VULKAN_HPP

#include <foe/wsi/export.h>
#include <foe/wsi/window.hpp>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <system_error>

/** Returns the required extensions for the current WSI implementation
 * @param pExtensionCount [out] Used to return the number of extensions returned
 * @return List of extension names, or nullptr on failure
 */
FOE_WSI_EXPORT auto foeWsiWindowGetVulkanExtensions(uint32_t *pExtensionCount,
                                                    char const ***pppExtensions) -> std::error_code;

/** Retrieves a Vulkan Surface object handle for the current active window
 * @param instance Vulkan instance that the surface will be associated with
 * @param pSurface [out] Pointer used to return the created Surface, if VK_SUCCESS is returned
 * @return VK_SUCCESS on success, an appropriate error otherwise
 */
FOE_WSI_EXPORT auto foeWsiWindowGetVkSurface(foeWsiWindow window,
                                             VkInstance instance,
                                             VkSurfaceKHR *pSurface) -> VkResult;

#endif // FOE_WSI_VULKAN_HPP