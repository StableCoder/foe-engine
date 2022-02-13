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

#include <foe/imgui/vk/render_graph_job_imgui.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/imgui/state.hpp>
#include <foe/imgui/vk/renderer.hpp>
#include <vk_error_code.hpp>

#include "error_code.hpp"

auto foeImGuiVkRenderUiJob(foeGfxVkRenderGraph renderGraph,
                           std::string_view name,
                           VkFence fence,
                           foeGfxVkRenderGraphResource renderTarget,
                           VkImageLayout finalLayout,
                           foeImGuiRenderer *pImguiRenderer,
                           foeImGuiState *pImguiState,
                           uint32_t frameIndex,
                           foeGfxVkRenderGraphResource *pResourcesOut) -> std::error_code {
    std::error_code errC;

    // Check that render target is a mutable image
    auto *pColourTargetImageData = (foeGfxVkGraphImageResource *)foeGfxVkGraphFindStructure(
        renderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pColourTargetImageData == nullptr)
        return FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE;
    if (!pColourTargetImageData->isMutable)
        return FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE;

    // Get the render target's previous layout
    auto *pColourTargetState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        renderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pColourTargetState == nullptr)
        return FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE;

    auto *pJob = new RenderGraphJob;
    *pJob = RenderGraphJob{
        .name = std::string{name},
        .required = false,
        .executeFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                         std::vector<VkSemaphore> const &waitSemaphores,
                         std::vector<VkSemaphore> const &signalSemaphores,
                         std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
            std::error_code errC;

            foeGfxVkGraphImageResource *pRenderTargetImage =
                reinterpret_cast<foeGfxVkGraphImageResource *>(renderTarget.pResourceData);

            VkRenderPass renderPass = foeGfxVkGetRenderPassPool(gfxSession)
                                          ->renderPass({VkAttachmentDescription{
                                              .format = pRenderTargetImage->format,
                                              .samples = VK_SAMPLE_COUNT_1_BIT,
                                              .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                              .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                              .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                              .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                              .initialLayout = pColourTargetState->layout,
                                              .finalLayout = finalLayout,
                                          }});

            VkFramebuffer framebuffer;

            { // Create Framebuffer
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = &pRenderTargetImage->view,
                    .width = pRenderTargetImage->extent.width,
                    .height = pRenderTargetImage->extent.height,
                    .layers = 1,
                };
                errC = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr,
                                           &framebuffer);
                if (errC)
                    return errC;

                foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                    vkDestroyFramebuffer(foeGfxVkGetDevice(session), framebuffer, nullptr);
                });
            }

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

            { // Setup common render viewport data
                VkViewport viewport{
                    .width = static_cast<float>(pRenderTargetImage->extent.width),
                    .height = static_cast<float>(pRenderTargetImage->extent.height),
                    .minDepth = 0.f,
                    .maxDepth = 1.f,
                };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{
                    .offset = VkOffset2D{},
                    .extent = pRenderTargetImage->extent,
                };
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                // vkDepthBias ??
            }

            { // Start RenderPass
                VkClearValue clear{
                    .color = {0.f, 0.5f, 1.f, 0.f},
                };
                VkRenderPassBeginInfo renderPassBI{
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = renderPass,
                    .framebuffer = framebuffer,
                    .renderArea =
                        {
                            .offset = {0, 0},
                            .extent = pRenderTargetImage->extent,
                        },
                    .clearValueCount = 1,
                    .pClearValues = &clear,
                };

                vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
            }

            if (!pImguiRenderer->initialized()) {
                errC = pImguiRenderer->initialize(gfxSession, VK_SAMPLE_COUNT_1_BIT, renderPass, 0);
                if (errC)
                    return errC;
            }

            pImguiRenderer->newFrame();
            pImguiState->runUI();
            pImguiRenderer->endFrame();

            errC = pImguiRenderer->update(frameIndex);
            if (errC)
                return errC;

            pImguiRenderer->draw(commandBuffer, frameIndex);

            vkCmdEndRenderPass(commandBuffer);
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

    // Resource Management
    auto *pFinalImageState = new foeGfxVkGraphImageState;
    *pFinalImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalLayout,
    };

    DeleteResourceDataCall deleteCalls{
        .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
            delete reinterpret_cast<foeGfxVkGraphImageState *>(pResource);
        },
        .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pFinalImageState),
    };

    // Add job to graph
    bool const resourcesInReadOnly = false;

    errC = foeGfxVkRenderGraphAddJob(renderGraph, pJob, 1, &renderTarget, &resourcesInReadOnly, 1,
                                     &deleteCalls, pResourcesOut);
    if (errC) {
        deleteCalls.deleteFn(deleteCalls.pResource);

        return errC;
    }

    // Outgoing resources
    pResourcesOut->pResourceState = reinterpret_cast<foeGfxVkGraphStructure *>(pFinalImageState);

    return errC;
}