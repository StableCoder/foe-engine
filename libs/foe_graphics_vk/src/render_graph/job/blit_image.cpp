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

#include <foe/graphics/vk/render_graph/job/blit_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

auto foeGfxVkBlitImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                std::string_view name,
                                VkFence fence,
                                RenderGraphResource srcImage,
                                VkImageLayout srcInitialLayout,
                                VkImageLayout srcFinalLayout,
                                RenderGraphResource dstImage,
                                VkImageLayout dstInitialLayout,
                                VkImageLayout dstFinalLayout) -> BlitJobUsedResources {
    auto *pJob = new RenderGraphJob;
    *pJob = RenderGraphJob{
        .name = std::string{name},
        .required = false,
        .executeFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                         std::vector<VkSemaphore> const &waitSemaphores,
                         std::vector<VkSemaphore> const &signalSemaphores,
                         std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
            std::error_code errC;

            RenderGraphResourceImage *pSrcImage =
                reinterpret_cast<RenderGraphResourceImage *>(srcImage.pResourceData);
            RenderGraphResourceImage *pDstImage =
                reinterpret_cast<RenderGraphResourceImage *>(dstImage.pResourceData);

            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;

            { // Create CommandPool
                VkCommandPoolCreateInfo poolCI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .queueFamilyIndex = 0,
                };
                errC = vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr,
                                           &commandPool);
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

            if (srcInitialLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = foeGfxVkDetermineAccessFlags(srcInitialLayout),
                    .dstAccessMask =
                        foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                    .oldLayout = srcInitialLayout,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = pSrcImage->image,
                    .subresourceRange = subresourceRange,
                };
                ++numBarriers;
            }
            if (dstInitialLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = foeGfxVkDetermineAccessFlags(dstInitialLayout),
                    .dstAccessMask =
                        foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
                    .oldLayout = dstInitialLayout,
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
                        .width = pSrcImage->extent.width,
                        .height = pSrcImage->extent.height,
                        .depth = 1,
                    },
            };

            vkCmdCopyImage(commandBuffer, pSrcImage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           pDstImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

            // Transition images layout/masks of outgoing
            numBarriers = 0;

            if (srcFinalLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                imgMemBarrier[numBarriers] = VkImageMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask =
                        foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
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
                    .srcAccessMask =
                        foeGfxVkDetermineAccessFlags(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
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
        },
    };

    std::array<RenderGraphResource const, 2> resourcesIn{srcImage, dstImage};
    std::array<bool const, 2> resourcesInReadOnly{true, false};
    std::array<RenderGraphResource, 2> resourcesOut{};

    foeGfxVkRenderGraphAddJob(renderGraph, pJob, 2, resourcesIn.data(), resourcesInReadOnly.data(),
                              0, nullptr, resourcesOut.data());

    BlitJobUsedResources consumedResources{
        .srcImage = resourcesOut[0],
        .dstImage = resourcesOut[1],
    };

    return consumedResources;
}