// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/queue_family.hpp>

#include <foe/graphics/type_defs.h>

VkQueue foeGfxTryGetQueue(foeGfxVkQueueFamily *pQueueFamily) {
    for (uint32_t i = 0; i < pQueueFamily->numQueues; ++i) {
        if (pQueueFamily->sync[i].try_lock()) {
            return pQueueFamily->queue[i];
        }
    }

    return VK_NULL_HANDLE;
}

VkQueue foeGfxGetQueue(foeGfxVkQueueFamily *pQueueFamily) {
    VkQueue queue = foeGfxTryGetQueue(pQueueFamily);

    while (queue == VK_NULL_HANDLE) {
        queue = foeGfxTryGetQueue(pQueueFamily);
    }

    return queue;
}

void foeGfxReleaseQueue(foeGfxVkQueueFamily *pQueueFamily, VkQueue queue) {
    for (uint32_t i = 0; i < pQueueFamily->numQueues; ++i) {
        if (pQueueFamily->queue[i] == queue) {
            pQueueFamily->sync[i].unlock();
        }
    }
}