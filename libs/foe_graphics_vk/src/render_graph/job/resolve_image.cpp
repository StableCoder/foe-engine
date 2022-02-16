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

#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include "../../error_code.hpp"

auto foeGfxVkResolveImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                   std::string_view name,
                                   VkFence fence,
                                   foeGfxVkRenderGraphResource srcImage,
                                   VkImageLayout srcFinalLayout,
                                   foeGfxVkRenderGraphResource dstImage,
                                   VkImageLayout dstFinalLayout,
                                   ResolveJobUsedResources *pResourcesOut) -> std::error_code {
    std::error_code errC;

    // Check that resources are images and the destination is mutable
    auto *pSrcImageData = (foeGfxVkGraphImageResource *)foeGfxVkGraphFindStructure(
        srcImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto *pDstImageData = (foeGfxVkGraphImageResource *)foeGfxVkGraphFindStructure(
        dstImage.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pSrcImageData == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE;
    if (pDstImageData == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE;
    if (!pDstImageData->isMutable)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE;

    // Get the last states of the images
    auto *pSrcImageState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        srcImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto *pDstImageState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        dstImage.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pSrcImageState == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE;
    if (pDstImageState == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE;

    // Proceed with the job
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
        std::error_code errC;

        foeGfxVkGraphImageResource *pSrcImage =
            reinterpret_cast<foeGfxVkGraphImageResource *>(srcImage.pResourceData);
        foeGfxVkGraphImageResource *pDstImage =
            reinterpret_cast<foeGfxVkGraphImageResource *>(dstImage.pResourceData);

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        { // Create CommandPool
            VkCommandPoolCreateInfo poolCI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = 0,
            };
            errC =
                vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr, &commandPool);
            if (errC)
                return errC;

            foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
            });
        }

        { // Create CommandBuffer
            VkCommandBufferAllocateInfo commandBufferAI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            errC = vkAllocateCommandBuffers(foeGfxVkGetDevice(gfxSession), &commandBufferAI,
                                            &commandBuffer);
            if (errC)
                return errC;
        }

        { // Start CommandBuffer
            VkCommandBufferBeginInfo commandBufferBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            errC = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
            if (errC)
                return errC;
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
                .image = pSrcImage->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }
        if (pDstImageState->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                // @todo replace with correctly determined masks for dynamic work
                .srcAccessMask = foeGfxVkDetermineAccessFlags(pDstImageState->layout),
                .dstAccessMask = foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                .oldLayout = pDstImageState->layout,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = pDstImage->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

        // Resolve Command
        VkImageResolve resolveRegion{
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
                    .width = pSrcImage->extent.width,
                    .height = pSrcImage->extent.height,
                    .depth = 1,
                },
        };

        vkCmdResolveImage(commandBuffer, pSrcImage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          pDstImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                          &resolveRegion);

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
                .image = pSrcImage->image,
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
                .image = pDstImage->image,
                .subresourceRange = subresourceRange,
            };
            ++numBarriers;
        }

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr,
                             numBarriers, imgMemBarrier);

        // End Command buffer and submit
        errC = vkEndCommandBuffer(commandBuffer);
        if (errC)
            return errC;

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
        errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return errC;
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

    DeleteResourceDataCall deleteCall{
        .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
            delete[] reinterpret_cast<foeGfxVkGraphImageState *>(pResource);
        },
        .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pFinalImageStates),
    };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResource const, 2> resourcesIn{srcImage, dstImage};
    std::array<bool const, 2> resourcesInReadOnly{true, false};
    foeGfxVkRenderGraphJob renderGraphJob;

    errC =
        foeGfxVkRenderGraphAddJob(renderGraph, 2, resourcesIn.data(), resourcesInReadOnly.data(), 1,
                                  &deleteCall, name, false, std::move(jobFn), &renderGraphJob);
    if (errC) {
        deleteCall.deleteFn(deleteCall.pResource);
        return errC;
    }

    // Outgoing resources
    *pResourcesOut = ResolveJobUsedResources{
        .srcImage =
            {
                .provider = renderGraphJob,
                .pResourceData = srcImage.pResourceData,
                .pResourceState = reinterpret_cast<foeGfxVkGraphStructure *>(pFinalImageStates),
            },
        .dstImage =
            {
                .provider = renderGraphJob,
                .pResourceData = dstImage.pResourceData,
                .pResourceState = reinterpret_cast<foeGfxVkGraphStructure *>(pFinalImageStates + 1),
            },
    };

    return FOE_GRAPHICS_VK_SUCCESS;
}