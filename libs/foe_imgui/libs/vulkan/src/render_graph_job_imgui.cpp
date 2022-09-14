// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imgui/vk/render_graph_job_imgui.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/session.h>
#include <foe/imgui/state.hpp>
#include <foe/imgui/vk/renderer.hpp>

#include "result.h"
#include "vk_result.h"

namespace {

void destroy_VkFramebuffer(VkFramebuffer framebuffer, foeGfxSession session) {
    vkDestroyFramebuffer(foeGfxVkGetDevice(session), framebuffer, nullptr);
}

void destroy_VkCommandPool(VkCommandPool commandPool, foeGfxSession session) {
    vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
}

} // namespace

foeResultSet foeImGuiVkRenderUiJob(foeGfxVkRenderGraph renderGraph,
                                   std::string_view name,
                                   VkFence fence,
                                   foeGfxVkRenderGraphResource renderTarget,
                                   VkImageLayout finalLayout,
                                   foeImGuiRenderer *pImguiRenderer,
                                   foeImGuiState *pImguiState,
                                   uint32_t frameIndex,
                                   foeGfxVkRenderGraphResource *pResourcesOut) {
    // Check that render target is a mutable image
    auto const *pColourTargetImageData =
        (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
            renderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pColourTargetImageData == nullptr)
        return to_foeResult(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE);
    if (!pColourTargetImageData->isMutable)
        return to_foeResult(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE);

    // Get the render target's previous layout
    auto const *pColourTargetState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        renderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pColourTargetState == nullptr)
        return to_foeResult(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE);

    // Job Data
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResultSet {
        VkResult vkResult;

        VkRenderPass renderPass = foeGfxVkGetRenderPassPool(gfxSession)
                                      ->renderPass({VkAttachmentDescription{
                                          .format = pColourTargetImageData->format,
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
                .pAttachments = &pColourTargetImageData->view,
                .width = pColourTargetImageData->extent.width,
                .height = pColourTargetImageData->extent.height,
                .layers = 1,
            };
            vkResult = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr,
                                           &framebuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_VkFramebuffer,
                                        (void *)framebuffer);
        }

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
                                        (PFN_foeGfxDelayedCall)destroy_VkCommandPool,
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

        { // Setup common render viewport data
            VkViewport viewport{
                .width = static_cast<float>(pColourTargetImageData->extent.width),
                .height = static_cast<float>(pColourTargetImageData->extent.height),
                .minDepth = 0.f,
                .maxDepth = 1.f,
            };
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{
                .offset = VkOffset2D{},
                .extent = pColourTargetImageData->extent,
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
                        .extent = pColourTargetImageData->extent,
                    },
                .clearValueCount = 1,
                .pClearValues = &clear,
            };

            vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
        }

        if (!pImguiRenderer->initialized()) {
            foeResultSet result =
                pImguiRenderer->initialize(gfxSession, VK_SAMPLE_COUNT_1_BIT, renderPass, 0);
            if (result.value != FOE_SUCCESS)
                return result;
        }

        pImguiRenderer->newFrame();
        pImguiState->runUI();
        pImguiRenderer->endFrame();

        foeResultSet result = pImguiRenderer->update(frameIndex);
        if (result.value != FOE_SUCCESS)
            return result;

        pImguiRenderer->draw(commandBuffer, frameIndex);

        vkCmdEndRenderPass(commandBuffer);
        vkResult = vkEndCommandBuffer(commandBuffer);
        if (vkResult)
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
    auto *pFinalImageState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalLayout,
    };
    if (pFinalImageState == nullptr)
        return to_foeResult(FOE_IMGUI_VK_ERROR_OUT_OF_MEMORY);

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pFinalImageState; };

    // Add job to graph
    bool const resourcesInReadOnly = false;
    foeGfxVkRenderGraphJob renderGraphJob;

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, 1, &renderTarget, &resourcesInReadOnly, freeDataFn,
                                  name, false, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {
        // Outgoing resources
        *pResourcesOut = {
            .provider = renderGraphJob,
            .pResourceData = renderTarget.pResourceData,
            .pResourceState =
                reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pFinalImageState),
        };
    }

    return result;
}