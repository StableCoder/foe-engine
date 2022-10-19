// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/present_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../result.h"
#include "../../vk_result.h"

#include <functional>
#include <vector>

struct foeGfxVkGraphSwapchainResource {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
    VkSwapchainKHR swapchain;
    uint32_t index;
};

foeResultSet foeGfxVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                   char const *pJobName,
                                                   VkFence fence,
                                                   char const *pResourceName,
                                                   VkSwapchainKHR swapchain,
                                                   uint32_t index,
                                                   VkImage image,
                                                   VkImageView view,
                                                   VkFormat format,
                                                   VkExtent2D extent,
                                                   VkImageLayout initialLayout,
                                                   VkSemaphore waitSemaphore,
                                                   foeGfxVkRenderGraphResource *pResourcesOut) {
    // Resource management
    struct PresentImageJobResources {
        foeGfxVkGraphSwapchainResource swapchainResource;
        foeGfxVkGraphImageResource swapchainImage;
        foeGfxVkGraphImageState swapchainImageState;
    };

    PresentImageJobResources *pJobResources = new (std::nothrow) PresentImageJobResources;
    if (pJobResources == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    pJobResources->swapchainResource = foeGfxVkGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
        .index = index,
    };

    pJobResources->swapchainImage = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = &pJobResources->swapchainResource,
        .name = pResourceName,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    pJobResources->swapchainImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = initialLayout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,
        .fence = fence,
    };

    foeResultSet result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .provider = renderGraphJob,
        .pResourceData =
            reinterpret_cast<foeGfxVkRenderGraphStructure *>(&pJobResources->swapchainImage),
        .pResourceState =
            reinterpret_cast<foeGfxVkRenderGraphStructure *>(&pJobResources->swapchainImageState),
    };

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

namespace {

void destroy_VkSemaphore(VkSemaphore semaphore, foeGfxSession session) {
    vkDestroySemaphore(foeGfxVkGetDevice(session), semaphore, nullptr);
}

} // namespace

foeResultSet foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                    char const *pJobName,
                                                    VkFence fence,
                                                    foeGfxVkRenderGraphResource swapchainResource) {
    // Check that the given resource is the correct type
    auto const *pSwapchainData = (foeGfxVkGraphSwapchainResource const *)foeGfxVkGraphFindStructure(
        swapchainResource.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN);

    if (pSwapchainData == nullptr)
        return to_foeResult(
            FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN);

    // Check that the image state is correct
    auto const *pImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        swapchainResource.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pImageState == nullptr)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE);

    if (pImageState->layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        std::abort();

    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &signalSemaphores) -> foeResultSet {
        VkResult vkResult;

        // If we have a fence, then we need to signal it
        VkPipelineStageFlags waitMask{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSemaphore signalSemaphore{VK_NULL_HANDLE};

        { // Create temporary semaphore
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            vkResult = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                         &signalSemaphore);
            if (vkResult)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_VkSemaphore,
                                        (void *)signalSemaphore);
        }

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = &waitMask,
            .signalSemaphoreCount = 1U,
            .pSignalSemaphores = &signalSemaphore,
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
        if (vkResult != VK_SUCCESS)
            std::abort();

        // Now get on with presenting the image
        foeGfxVkGraphSwapchainResource const *pSwapchainResource =
            reinterpret_cast<foeGfxVkGraphSwapchainResource const *>(
                swapchainResource.pResourceData->pNext);

        VkResult presentRes{VK_SUCCESS};
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1U,
            .pWaitSemaphores = &signalSemaphore,
            .swapchainCount = 1U,
            .pSwapchains = &pSwapchainResource->swapchain,
            .pImageIndices = &pSwapchainResource->index,
            .pResults = &presentRes,
        };

        queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        vkResult = vkQueuePresentKHR(queue, &presentInfo);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR) {
            // The associated window has been resized, will be fixed for the next frame
            vkResult = VK_SUCCESS;
        } else if (vkResult != VK_SUCCESS) {
            return vk_to_foeResult(vkResult);
        }

        if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR) {
            // The associated window has been resized, will be fixed for the next frame
            vkResult = VK_SUCCESS;
        } else if (presentRes != VK_SUCCESS) {
            return vk_to_foeResult(presentRes);
        }

        return vk_to_foeResult(vkResult);
    };

    // Add job to graph
    bool const resourceReadOnly = false;
    foeGfxVkRenderGraphJob renderGraphJob;

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResourcesIn = &swapchainResource,
        .pResourcesInReadOnly = &resourceReadOnly,
        .name = pJobName,
        .required = true,
        .fence = fence,
    };

    return foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, std::move(jobFn), {}, &renderGraphJob);
}