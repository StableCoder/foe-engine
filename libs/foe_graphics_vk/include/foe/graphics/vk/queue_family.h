// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_QUEUE_FAMILY_H
#define FOE_GRAPHICS_VK_QUEUE_FAMILY_H

#include <foe/graphics/export.h>
#include <foe/handle.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxVkQueueFamily)

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available
 * @note Non-blocking
 */
FOE_GFX_EXPORT
VkQueue foeGfxTryGetQueue(foeGfxVkQueueFamily queueFamily);

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available (No
 * queues in family)
 * @note Blocking
 */
FOE_GFX_EXPORT
VkQueue foeGfxGetQueue(foeGfxVkQueueFamily queueFamily);

/**
 * @brief Releases a queue to the family so that it can be used elsewhere
 * @param pQueueFamily Queue family to get release a queue to
 * @param queue The specific queue being released.
 * @warning The queue MUST belong to the queue family, otherwise nothing will happen!
 */
FOE_GFX_EXPORT
void foeGfxReleaseQueue(foeGfxVkQueueFamily queueFamily, VkQueue queue);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_QUEUE_FAMILY_H