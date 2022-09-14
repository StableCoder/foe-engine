// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/queue_family.h>

#include "queue_family.hpp"

extern "C" VkQueue foeGfxTryGetQueue(foeGfxVkQueueFamily queueFamily) {
    QueueFamily *pQueueFamily = queue_family_from_handle(queueFamily);

    for (uint32_t i = 0; i < pQueueFamily->numQueues; ++i) {
        if (pQueueFamily->sync[i].try_lock()) {
            return pQueueFamily->queue[i];
        }
    }

    return VK_NULL_HANDLE;
}

extern "C" VkQueue foeGfxGetQueue(foeGfxVkQueueFamily queueFamily) {
    VkQueue queue = foeGfxTryGetQueue(queueFamily);

    while (queue == VK_NULL_HANDLE) {
        queue = foeGfxTryGetQueue(queueFamily);
    }

    return queue;
}

extern "C" void foeGfxReleaseQueue(foeGfxVkQueueFamily queueFamily, VkQueue queue) {
    QueueFamily *pQueueFamily = queue_family_from_handle(queueFamily);

    for (uint32_t i = 0; i < pQueueFamily->numQueues; ++i) {
        if (pQueueFamily->queue[i] == queue) {
            pQueueFamily->sync[i].unlock();
        }
    }
}
