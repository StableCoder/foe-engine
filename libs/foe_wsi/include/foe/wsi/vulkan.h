// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_WSI_VULKAN_H
#define FOE_WSI_VULKAN_H

#include <foe/result.h>
#include <foe/wsi/export.h>
#include <foe/wsi/window.h>
#include <vulkan/vulkan.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Returns the required extensions for the current WSI implementation
 * @param pExtensionCount [out] Used to return the number of extensions returned
 * @param pppExtenstions [out] List of extension names
 * @return FOE_WSI_SUCCESS on success, an appropriate error code otherwise.
 */
FOE_WSI_EXPORT
foeResultSet foeWsiWindowGetVulkanExtensions(uint32_t *pExtensionCount,
                                             char const ***pppExtensions);

/** Retrieves a Vulkan Surface object handle for the current active window
 * @param instance Vulkan instance that the surface will be associated with
 * @param pSurface [out] Pointer used to return the created Surface, if VK_SUCCESS is returned
 * @return VK_SUCCESS on success, an appropriate error otherwise
 */
FOE_WSI_EXPORT
foeResultSet foeWsiWindowGetVkSurface(foeWsiWindow window,
                                      VkInstance instance,
                                      VkSurfaceKHR *pSurface);

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_VULKAN_H