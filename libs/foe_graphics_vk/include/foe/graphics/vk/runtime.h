// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RUNTIME_H
#define FOE_GRAPHICS_VK_RUNTIME_H

#include <foe/graphics/export.h>
#include <foe/graphics/runtime.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Creates a graphics runtime using the Vulkan API
 * @param pApplicationName is NULL or is a pointer to a null-terminated UTF-8 string containing the
 * name of the application.
 * @param applicationVersion is an unsigned integer variable containing the developer-supplied
 * version number of the application.
 * @param applicationApiVersion is supposed to be the highest version of Vulkan that the application
 * is designed to use, and thus the version the runtime will be created with
 * @param layerCount is the number of Vulkan layers to enable
 * @param ppLayerNames is a pointer to an array of layerCount null-terminated UTF-8 strings
 * containing the names of layers to enable in the created runtime.
 * @param extensionCount is the number of Vulkan extensions to enable
 * @param ppExtensionNames is a pointer to an array of layerCount null-terminated UTF-8 strings
 * containing the names of extesnions to enable in the created runtime.
 * @param validation indicates whether to enable the VK_LAYER_KHRONOS_validation layer
 * @param debugLogging indicates whether to enable debug logging.
 * @param pRuntime points to a foeGfxRuntime handle in which the resulting runtime is returned.
 * @return FOE_GFX_VK_SUCCESS on success, or an appropriate error otherwise.
 */
FOE_GFX_EXPORT
foeResultSet foeGfxVkCreateRuntime(char const *pApplicationName,
                                   uint32_t applicationVersion,
                                   uint32_t applicationApiVersion,
                                   uint32_t layerCount,
                                   char const *const *ppLayerNames,
                                   uint32_t extensionCount,
                                   char const *const *ppExtensionNames,
                                   bool validation,
                                   bool debugLogging,
                                   foeGfxRuntime *pRuntime);

/** @brief Enumerate the enabled layers for the given runtime
 * @param runtime is the handle to the runtime whose layers will be queried.
 * @param pLayerNamesLength is a pointer to an integer related to the size of pLayarNames, as
 * described below.
 * @param pLayerNames is either NULL or a pointer to an character array.
 * @return FOE_GRAPHICS_VK_SUCCESS on success, FOE_GRAPHICS_VK_INCOMPLETE if not all names could be
 * returned
 *
 * If pLayerNames is NULL, then the size required to return all layer names is returned int
 * pLayerNamesLength. Otherwise, pLayerNamesLength must point to a variable set by the user to the
 * size of the pLayerNames array, and on return the variable is overwritten with the characters
 * actually written to pLayerNames. If pLayerNamesLength is less than the total size required to
 * return all names, at most pLayerNamesLength is written, and FOE_GFX_VK_INCOMPLETE will be
 * returned instead of FOE_GFX_VK_SUCCESS, to indicate that not all names were returned.
 */
FOE_GFX_EXPORT
foeResultSet foeGfxVkEnumerateRuntimeLayers(foeGfxRuntime runtime,
                                            uint32_t *pLayerNamesLength,
                                            char *pLayerNames);

/** @brief Enumerate the enabled extensions for the given runtime
 * @param runtime is the handle to the runtime whose extensions will be queried.
 * @param pExtensionNamesLength is a pointer to an integer related to the size of pExtensionNames,
 * as described below.
 * @param pExtensionNames is either NULL or a pointer to an character array.
 * @return FOE_GRAPHICS_VK_SUCCESS on success, FOE_GRAPHICS_VK_INCOMPLETE if not all names could be
 * returned
 *
 * If pExtensionNames is NULL, then the size required to return all layer names is returned int
 * pExtensionNamesLength. Otherwise, pExtensionNamesLength must point to a variable set by
 * the user to the size of the pExtensionNames array, and on return the variable is overwritten
 * with the characters actually written to pExtensionNames. If pExtensionNamesLength is
 * less than the total size required to return all names, at most pExtensionNamesLength is
 * written, and FOE_GFX_VK_INCOMPLETE will be returned instead of FOE_GFX_VK_SUCCESS, to indicate
 * that not all names were returned.
 */
FOE_GFX_EXPORT
foeResultSet foeGfxVkEnumerateRuntimeExtensions(foeGfxRuntime runtime,
                                                uint32_t *pExtensionNamesLength,
                                                char *pExtensionNames);

/** @brief Returns the API version a Vulkan graphics runtime was created with
 * @param runtime Runtime to query
 * @return A uint32_t representing the API version
 */
FOE_GFX_EXPORT
uint32_t foeGfxVkEnumerateApiVersion(foeGfxRuntime runtime);

/** @brief Returns the Vulkan instance handle
 * @param runtime is the handle to the Vukan-based graphics runtime
 * @return The VkInstance handle
 */
FOE_GFX_EXPORT
VkInstance foeGfxVkGetRuntimeInstance(foeGfxRuntime runtime);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_RUNTIME_H