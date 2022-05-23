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

#ifndef FOE_GRAPHICS_VK_SESSION_HPP
#define FOE_GRAPHICS_VK_SESSION_HPP

#include <foe/error_code.h>
#include <foe/graphics/builtin_descriptor_sets.h>
#include <foe/graphics/export.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

class foeGfxVkRenderPassPool;
class foeGfxVkFragmentDescriptorPool;
class foeGfxVkPipelinePool;

/** @brief Creates a graphics session using the Vulkan API
 * @param runtime is a handle to the graphics runtime to be created on
 * @param vkPhysicalDevice must be a physical device handle retrieved using the given runtime
 * @param layerCount is the number of Vulkan layers to enable
 * @param ppLayerNames is a pointer to an array of layerCount null-terminated UTF-8 strings
 * containing the names of layers to enable in the created runtime.
 * @param extensionCount is the number of Vulkan extensions to enable
 * @param ppExtensionNames is a pointer to an array of layerCount null-terminated UTF-8 strings
 * containing the names of extesnions to enable in the created runtime.
 * @param pBasicFeatures is either NULL or a pointer to the set of Vulkan 1.0 features to enable.
 * These can also be passed in VkPhysicalDeviceFeatures2 to pFeatures.
 * @param pFeatures is either NULL or a pointer to a chain of Vulkan feature structs. If a Vulkan
 * feature set is beyond the Vulkan API version the runtime was created with, then
 * FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT is returned. If a struct in this
 * set doesn't have an implementation compiled for it or is otherwise unknown,
 * FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT is returned.
 * @param pSession points to a foeGfxSession handle in which the resulting session is returned.
 * @return FOE_GFX_VK_SUCCESS on success, or an appropriate error otherwise.
 *
 * The pFeatures struct chain must include the types that include an sType and pNext void pointer,
 * such as VkPhysicalDeviceFeatures2 or VkPhysicalDeviceVulkan11Features, and as many of these
 * structs can be chained as desired.
 */
FOE_GFX_EXPORT foeResult foeGfxVkCreateSession(foeGfxRuntime runtime,
                                               VkPhysicalDevice vkPhysicalDevice,
                                               uint32_t layerCount,
                                               char const *const *ppLayerNames,
                                               uint32_t extensionCount,
                                               char const *const *ppExtensionNames,
                                               VkPhysicalDeviceFeatures const *pBasicFeatures,
                                               void const *pFeatures,
                                               foeGfxSession *pSession);

/** @brief Enumerate the enabled layers for the given session
 * @param session is the handle to the session whose layers will be queried.
 * @param pLayerNamesLength is a pointer to an integer related to the size of pLayerNames, as
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
FOE_GFX_EXPORT foeResult foeGfxVkEnumerateSessionLayers(foeGfxSession session,
                                                        uint32_t *pLayerNamesLength,
                                                        char *pLayerNames);

/** @brief Enumerate the enabled extensions for the given session
 * @param session is the handle to the session whose extensions will be queried.
 * @param pExtensionNamesLength is a pointer to an integer related to the size of pExtensionNames,
 * as described below.
 * @param pExtensionNames is either NULL or a pointer to an character array.
 *
 * If pExtensionNames is NULL, then the size required to return all layer names is returned int
 * pExtensionNamesLength. Otherwise, pExtensionNamesLength must point to a variable set by
 * the user to the size of the pExtensionNames array, and on return the variable is overwritten
 * with the characters actually written to pExtensionNames. If pExtensionNamesLength is
 * less than the total size required to return all names, at most pExtensionNamesLength is
 * written, and FOE_GFX_VK_INCOMPLETE will be returned instead of FOE_GFX_VK_SUCCESS, to indicate
 * that not all names were returned.
 */
FOE_GFX_EXPORT foeResult foeGfxVkEnumerateSessionExtensions(foeGfxSession session,
                                                            uint32_t *pExtensionNamesLength,
                                                            char *pExtensionNames);

/** @brief Fills out given feature structs with then enabled session's features
 * @param session is the handle to the session whose features will be queried.
 *@param pBasicFeatures is either NULL or a pointer to a struct to fill in with features.
 * @param pFeatures is either NULL or a pointer to a chain of Vulkan feature structs.
 *
 * The pFeatures struct chain must include the types that include an sType and pNext void pointer,
 * such as VkPhysicalDeviceFeatures2 or VkPhysicalDeviceVulkan11Features, and as many of these
 * structs can be chained as desired.
 */
FOE_GFX_EXPORT void foeGfxVkEnumerateSessionFeatures(foeGfxSession session,
                                                     VkPhysicalDeviceFeatures const *pBasicFeatures,
                                                     void *pFeatures);

FOE_GFX_EXPORT VkInstance foeGfxVkGetInstance(foeGfxSession session);
FOE_GFX_EXPORT VkPhysicalDevice foeGfxVkGetPhysicalDevice(foeGfxSession session);
FOE_GFX_EXPORT VkDevice foeGfxVkGetDevice(foeGfxSession session);
FOE_GFX_EXPORT VmaAllocator foeGfxVkGetAllocator(foeGfxSession session);

FOE_GFX_EXPORT uint32_t foeGfxVkGetBestQueue(foeGfxSession session, VkQueueFlags flags);

#include <foe/graphics/vk/queue_family.hpp>
FOE_GFX_EXPORT foeGfxVkQueueFamily *getFirstQueue(foeGfxSession session);

FOE_GFX_EXPORT auto foeGfxVkGetDummySet(foeGfxSession session) -> VkDescriptorSet;

FOE_GFX_EXPORT auto foeGfxVkGetBuiltinLayout(foeGfxSession session,
                                             foeBuiltinDescriptorSetLayoutFlags builtinLayout)
    -> VkDescriptorSetLayout;

FOE_GFX_EXPORT auto foeGfxVkGetBuiltinSetLayoutIndex(
    foeGfxSession session, foeBuiltinDescriptorSetLayoutFlags builtinLayout) -> uint32_t;

FOE_GFX_EXPORT auto foeGfxVkGetRenderPassPool(foeGfxSession session) -> foeGfxVkRenderPassPool *;

FOE_GFX_EXPORT auto foeGfxVkGetFragmentDescriptorPool(foeGfxSession session)
    -> foeGfxVkFragmentDescriptorPool *;

FOE_GFX_EXPORT auto foeGfxVkGetPipelinePool(foeGfxSession session) -> foeGfxVkPipelinePool *;

FOE_GFX_EXPORT auto foeGfxVkGetSupportedMSAA(foeGfxSession session) -> VkSampleCountFlags;
FOE_GFX_EXPORT auto foeGfxVkGetMaxSupportedMSAA(foeGfxSession session) -> VkSampleCountFlags;

#endif // FOE_GRAPHICS_VK_SESSION_HPP