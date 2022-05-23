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

#ifndef FOE_GRAPHICS_VK_QUEUE_FAMILY_HPP
#define FOE_GRAPHICS_VK_QUEUE_FAMILY_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/type_defs.h>
#include <vulkan/vulkan.h>

#include <array>
#include <mutex>

struct foeGfxVkQueueFamily {
    VkQueueFlags flags = 0;
    uint32_t family;
    uint32_t numQueues = 0;

    std::array<std::mutex, MaxQueuesPerFamily> sync;
    std::array<VkQueue, MaxQueuesPerFamily> queue;
};

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available
 * @note Non-blocking
 */
FOE_GFX_EXPORT VkQueue foeGfxTryGetQueue(foeGfxVkQueueFamily *pQueueFamily);

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available (No
 * queues in family)
 * @note Blocking
 */
FOE_GFX_EXPORT VkQueue foeGfxGetQueue(foeGfxVkQueueFamily *pQueueFamily);

/**
 * @brief Releases a queue to the family so that it can be used elsewhere
 * @param pQueueFamily Queue family to get release a queue to
 * @param queue The specific queue being released.
 * @warning The queue MUST belong to the queue family, otherwise nothing will happen!
 */
FOE_GFX_EXPORT void foeGfxReleaseQueue(foeGfxVkQueueFamily *pQueueFamily, VkQueue queue);

#endif // FOE_GRAPHICS_VK_QUEUE_FAMILY_HPP