// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "queue_family.hpp"

SecuredQueue::SecuredQueue() : lock(), queue(VK_NULL_HANDLE) {}

SecuredQueue::SecuredQueue(VkQueue queue, std::unique_lock<std::mutex> &&lock) :
    lock(std::move(lock)), queue(queue) {}

void SecuredQueue::release() {
    queue = VK_NULL_HANDLE;
    lock.unlock();
}

SecuredQueue foeGfxTryGetQueue(QueueFamily *pQueueFamily) {
    for (uint32_t i = 0; i < pQueueFamily->numQueues; ++i) {
        std::unique_lock<std::mutex> lock(pQueueFamily->pQueues[i].sync, std::try_to_lock);

        if (lock) {
            return SecuredQueue(pQueueFamily->pQueues[i].queue, std::move(lock));
        }
    }

    return SecuredQueue();
}

SecuredQueue foeGfxGetQueue(QueueFamily *pQueueFamily) {
    SecuredQueue queue;

    do {
        queue = foeGfxTryGetQueue(pQueueFamily);
    } while (!queue);

    return std::move(queue);
}
