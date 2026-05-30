// Copyright (C) 2022-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef QUEUE_FAMILY_HPP
#define QUEUE_FAMILY_HPP

#include <foe/graphics/type_defs.h>
#include <foe/handle.h>
#include <vulkan/vulkan.h>

#include <mutex>

struct QueueData {
    std::mutex sync;
    VkQueue queue;
};

struct QueueFamily {
    VkQueueFlags flags;
    uint32_t family;
    uint32_t numQueues;
    QueueData *pQueues;
};

struct SecuredQueue {
    SecuredQueue();
    SecuredQueue(VkQueue queue, std::unique_lock<std::mutex> &&lock);

    void release();

    operator bool() const { return queue != VK_NULL_HANDLE; }
    operator VkQueue() const { return queue; }

  private:
    std::unique_lock<std::mutex> lock;
    VkQueue queue;
};

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available
 * @note Non-blocking
 */
SecuredQueue foeGfxTryGetQueue(QueueFamily *pQueueFamily);

/**
 * @brief Attempts to lock and return an available command queue
 * @param pQueueFamily Queue family to get a queue for
 * @return A valid VkQueue if one could be secured, VK_NULL_HANDLE if no queue is available (No
 * queues in family)
 * @note Blocking
 */
SecuredQueue foeGfxGetQueue(QueueFamily *pQueueFamily);

#endif // QUEUE_FAMILY_HPP