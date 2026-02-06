// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef PER_FRAME_DATA_HPP
#define PER_FRAME_DATA_HPP

#include <foe/split_thread_pool.h>
#include <vulkan/vulkan.h>

#include "vk_result.h"

#include <array>
#include <queue>

struct OnFrameCompleteTask {
    PFN_foeTask pfnTask;
    void *pTaskData;
};

struct PerFrameData {
    // if frame is in-progress
    bool active;

    VkSemaphore preGraph;
    VkSemaphore postGraph;

    VkFence frameComplete;
    std::queue<OnFrameCompleteTask> onFrameCompleteTasks;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    std::array<VkCommandBuffer, 2> xrCommandBuffers;

    foeResultSet create(VkDevice device) noexcept {
        VkResult vkResult{VK_SUCCESS};

        // Semaphores
        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        vkResult = vkCreateSemaphore(device, &semaphoreCI, nullptr, &preGraph);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);
        vkResult = vkCreateSemaphore(device, &semaphoreCI, nullptr, &postGraph);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // Fences
        VkFenceCreateInfo fenceCI{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        vkResult = vkCreateFence(device, &fenceCI, nullptr, &frameComplete);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // Command Pools
        VkCommandPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        };
        vkResult = vkCreateCommandPool(device, &poolCI, nullptr, &commandPool);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // Command Buffers
        VkCommandBufferAllocateInfo commandBufferAI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        vkResult = vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        commandBufferAI.commandBufferCount = 2;
        vkResult = vkAllocateCommandBuffers(device, &commandBufferAI, xrCommandBuffers.data());
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        return vk_to_foeResult(vkResult);
    }

    void destroy(VkDevice device) {
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyFence(device, frameComplete, nullptr);

        vkDestroySemaphore(device, postGraph, nullptr);
        vkDestroySemaphore(device, preGraph, nullptr);
    }
};

#endif // PER_FRAME_DATA_HPP