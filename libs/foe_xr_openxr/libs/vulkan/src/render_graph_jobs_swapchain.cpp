// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "log.hpp"
#include "result.h"
#include "vk_result.h"
#include "xr_result.h"

struct foeOpenXrRenderGraphSwapchainResource {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
    XrSwapchain swapchain;
};

namespace {

void destroy_VkSemaphore(VkSemaphore semaphore, foeGfxSession session) {
    vkDestroySemaphore(foeGfxVkGetDevice(session), semaphore, nullptr);
}

} // namespace

foeResultSet foeOpenXrVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                      char const *pJobName,
                                                      VkFence fence,
                                                      char const *pResourceName,
                                                      XrSwapchain swapchain,
                                                      VkImage image,
                                                      VkImageView view,
                                                      VkFormat format,
                                                      VkExtent2D extent,
                                                      VkImageLayout layout,
                                                      foeGfxVkRenderGraphResource *pResourcesOut) {
    // Add the job that waits on the timeline semaphore and signals any dependent jobs to start
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResultSet {
        // Create the timeline semaphore
        VkSemaphoreTypeCreateInfo timelineCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };

        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineCI,
            .flags = 0,
        };

        VkSemaphore timelineSemaphore;
        VkResult vkResult = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                              &timelineSemaphore);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                    (PFN_foeGfxDelayedCall)destroy_VkSemaphore,
                                    (void *)timelineSemaphore);

        addCpuFnFn([=]() {
            XrSwapchainImageWaitInfo waitInfo{
                .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
                .timeout = 10000, // In nanoseconds (0.01 ms)
            };

            XrResult xrResult{XR_TIMEOUT_EXPIRED};

            do {
                xrResult = xrWaitSwapchainImage(swapchain, &waitInfo);
            } while (xrResult == XR_TIMEOUT_EXPIRED);

            if (xrResult != XR_SUCCESS && xrResult != XR_SESSION_LOSS_PENDING) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                XrResultToString(xrResult, buffer);
                FOE_LOG(foeOpenXrVk, FOE_LOG_LEVEL_ERROR, "xrWaitSwapchainImage failed: {}",
                        buffer);
            } else {
                VkSemaphoreSignalInfo signalInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                    .pNext = nullptr,
                    .semaphore = timelineSemaphore,
                    .value = 1,
                };

                vkSignalSemaphore(foeGfxVkGetDevice(gfxSession), &signalInfo);
            }

            XrSwapchainImageReleaseInfo releaseInfo{
                .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
            };

            xrResult = xrReleaseSwapchainImage(swapchain, &releaseInfo);
            if (xrResult != XR_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                XrResultToString(xrResult, buffer);
                FOE_LOG(foeOpenXrVk, FOE_LOG_LEVEL_FATAL, "xrReleaseSwapchainImage error: {}",
                        buffer)
            }
        });

        // Submit
        const uint64_t waitValue = 1;
        VkTimelineSemaphoreSubmitInfo timelineSI{
            .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .waitSemaphoreValueCount = 1,
            .pWaitSemaphoreValues = &waitValue,
        };

        VkPipelineStageFlags waitMask{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSemaphore waitSemaphores{timelineSemaphore};

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = &timelineSI,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSemaphores,
            .pWaitDstStageMask = &waitMask,
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return vk_to_foeResult(vkResult);
    };

    // Resource management
    auto *pSwapchain = new (std::nothrow) foeOpenXrRenderGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
    };

    auto *pImage = new (std::nothrow) foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = pSwapchain,
        .name = pResourceName,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    auto *pImageState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        if (pSwapchain)
            delete pSwapchain;
        if (pImage)
            delete pImage;
        if (pImageState)
            delete pImageState;
    };

    if (pSwapchain == nullptr || pImage == nullptr || pImageState == nullptr) {
        freeDataFn();
        return to_foeResult(FOE_OPENXR_VK_ERROR_OUT_OF_MEMORY);
    }

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, 0, nullptr, nullptr, freeDataFn, pJobName, true,
                                  std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        // If we couldn't add it to the render graph, delete all heap data now
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .provider = renderGraphJob,
        .pResourceData = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImage),
        .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImageState),
    };

    return to_foeResult(FOE_OPENXR_VK_SUCCESS);
}
