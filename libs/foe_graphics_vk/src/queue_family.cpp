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

#include <foe/graphics/vk/queue_family.hpp>

#include <foe/graphics/type_defs.hpp>

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