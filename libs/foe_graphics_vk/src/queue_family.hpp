// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef QUEUE_FAMILY_HPP
#define QUEUE_FAMILY_HPP

#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/queue_family.h>
#include <vulkan/vulkan.h>

#include <array>
#include <mutex>

struct QueueFamily {
    VkQueueFlags flags = 0;
    uint32_t family;
    uint32_t numQueues = 0;

    std::array<std::mutex, FOE_GRAPHICS_MAX_QUEUES_PER_FAMILY> sync;
    std::array<VkQueue, FOE_GRAPHICS_MAX_QUEUES_PER_FAMILY> queue;
};

FOE_DEFINE_HANDLE_CASTS(queue_family, QueueFamily, foeGfxVkQueueFamily)

#endif // QUEUE_FAMILY_HPP