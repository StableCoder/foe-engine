/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_GRAPHICS_VK_RUNTIME_HPP
#define FOE_GRAPHICS_VK_RUNTIME_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/runtime.hpp>
#include <vulkan/vulkan.h>

#include <string>
#include <system_error>
#include <vector>

/** @brief Creates a graphics runtime using the Vulkan API
 * @param pApplicationName is NULL or is a pointer to a null-terminated UTF-8 string containing the
 * name of the application.
 * @param applicationVersion is an unsigned integer variable containing the developer-supplied
 * version number of the application.
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
FOE_GFX_EXPORT std::error_code foeGfxVkCreateRuntime(char const *pApplicationName,
                                                     uint32_t applicationVersion,
                                                     uint32_t layerCount,
                                                     char const *const *ppLayerNames,
                                                     uint32_t extensionCount,
                                                     char const *const *ppExtensionNames,
                                                     bool validation,
                                                     bool debugLogging,
                                                     foeGfxRuntime *pRuntime);

/** @brief Enumerate the enabled layers for the given runtime
 * @param runtime is the handle to the runtime whose layers will be queried.
 * @param pLayerNamesLength is a pointer to an integer related to the size of pLayaerNames, as
 * described below.
 * @param pLayerNames is either NULL or a pointer to an character array.
 *
 * If pLayerNames is NULL, then the size required to return all layer names is returned int
 * pLayerNamesLength. Otherwise, pLayerNamesLength must point to a variable set by the user to the
 * size of the pLayerNames array, and on return the variable is overwritten with the characters
 * actually written to pLayerNames. If pLayerNamesLength is less than the total size required to
 * return all names, at most pLayerNamesLength is written, and FOE_GFX_VK_INCOMPLETE will be
 * returned instead of FOE_GFX_VK_SUCCESS, to indicate that not all names were returned.
 */
FOE_GFX_EXPORT std::error_code foeGfxVkEnumerateRuntimeLayers(foeGfxRuntime runtime,
                                                              uint32_t *pLayerNamesLength,
                                                              char *pLayerNames);

/** @brief Enumerate the enabled extensions for the given runtime
 * @param runtime is the handle to the runtime whose extensions will be queried.
 * @param pExtensionsNamesLength is a pointer to an integer related to the size of pLayaerNames, as
 * described below.
 * @param pExtensionsNames is either NULL or a pointer to an character array.
 *
 * If pExtensionsNames is NULL, then the size required to return all layer names is returned int
 * pExtensionsNamesLength. Otherwise, pExtensionsNamesLength must point to a variable set by the
 * user to the size of the pExtensionsNames array, and on return the variable is overwritten with
 * the characters actually written to pExtensionsNames. If pExtensionsNamesLength is less than the
 * total size required to return all names, at most pExtensionsNamesLength is written, and
 * FOE_GFX_VK_INCOMPLETE will be returned instead of FOE_GFX_VK_SUCCESS, to indicate that not all
 * names were returned.
 */
FOE_GFX_EXPORT std::error_code foeGfxVkEnumerateRuntimeExtensions(foeGfxRuntime runtime,
                                                                  uint32_t *pExtensionNamesLength,
                                                                  char *pExtensionNames);

/** @brief Returns the Vulkan instance handle
 * @param runtime is the handle to the Vukan-based graphics runtime
 * @return The VkInstance handle
 */
FOE_GFX_EXPORT VkInstance foeGfxVkGetInstance(foeGfxRuntime runtime);

#endif // FOE_GRAPHICS_VK_RUNTIME_HPP