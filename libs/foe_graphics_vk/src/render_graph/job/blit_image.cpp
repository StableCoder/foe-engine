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

#include <foe/graphics/vk/render_graph/job/blit_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>

#include "../../result.h"
#include "../../vk_result.h"

namespace {

void cleanupOldCommandPool(VkCommandPool commandPool, foeGfxSession session) {
    vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
}

} // namespace

foeResult foeGfxVkBlitImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                     std::string_view name,
                                     VkFence fence,
                                     foeGfxVkRenderGraphResource srcImage,
                                     VkImageLayout srcFinalLayout,
                                     foeGfxVkRenderGraphResource dstImage,
                                     VkImageLayout dstFinalLayout,
                                     BlitJobUsedResources *pResourcesOut) {
    // Check that resources are correct types
    auto const *pSrcImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        srcImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDstImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        dstImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pSrcImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE);
    if (pDstImageData == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE);
    if (!pDstImageData->isMutable)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE);

    // Get the last states of the images
    auto const *pSrcImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        srcImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto const *pDstImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        dstImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pSrcImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE);
    if (pDstImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE);

    // Proceed with job
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResult {
        VkResult vkResult;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        { // Create CommandPool
            VkCommandPoolCreateInfo poolCI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = 0,
            };
            vkResult =
                vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr, &commandPool);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)cleanupOldCommandPool,
                                        (void *)commandPool);
        }

        { // Create CommandBuffer
            VkCommandBufferAllocateInfo commandBufferAI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkResult = vkAllocateCommandBuffers(foeGfxVkGetDevice(gfxSession), &commandBufferAI,
                                                &commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        { // Start CommandBuffer
            VkCommandBufferBeginInfo commandBufferBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        // Transition image layout/mask states of incoming
        VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .layerCount = 1,
        };
        uint32_t numBarriers = 0;
        VkImageMemoryBarrier imgMemBarrier[2] = {};

        if (pSrcImageState->layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(pSrcImageState->layout),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                .oldLayout = pSrcImageState->layout,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pSrcImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }
        if (pDstImageState->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(pDstImageState->layout),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                .oldLayout = pDstImageState->layout,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pDstImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

        // Copy Command
        VkImageCopy imageCopy{
            .srcSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .srcOffset = {},
            .dstSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .dstOffset = {},
            .extent =
                {
                    .width = pSrcImageData->extent.width,
                    .height = pSrcImageData->extent.height,
                    .depth = 1,
                },
        };

        vkCmdCopyImage(commandBuffer, pSrcImageData->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       pDstImageData->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

        // Transition images layout/masks of outgoing
        numBarriers = 0;

        if (srcFinalLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(srcFinalLayout),
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .newLayout = srcFinalLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pSrcImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }
        if (dstFinalLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(dstFinalLayout),
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = dstFinalLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pDstImageData->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

        // End Command buffer and submit
        vkResult = vkEndCommandBuffer(commandBuffer);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // Job Submission
        std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitMasks.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return vk_to_foeResult(vkResult);
    };

    // Resource Management
    auto *pFinalImageStates = new foeGfxVkGraphImageState[2];
    pFinalImageStates[0] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = srcFinalLayout,
    };
    pFinalImageStates[1] = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = dstFinalLayout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete[] pFinalImageStates; };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResource const, 2> resourcesIn{srcImage, dstImage};
    std::array<bool const, 2> resourcesInReadOnly{true, false};
    foeGfxVkRenderGraphJob renderGraphJob;

    foeResult result =
        foeGfxVkRenderGraphAddJob(renderGraph, 2, resourcesIn.data(), resourcesInReadOnly.data(),
                                  freeDataFn, name, false, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {

        // Outgoing resources
        *pResourcesOut = BlitJobUsedResources{
            .srcImage =
                {
                    .provider = renderGraphJob,
                    .pResourceData = srcImage.pResourceData,
                    .pResourceState =
                        reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pFinalImageStates),
                },
            .dstImage =
                {
                    .provider = renderGraphJob,
                    .pResourceData = dstImage.pResourceData,
                    .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(
                        pFinalImageStates + 1),
                },
        };
    }

    return result;
}