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
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
    };

    pJobResources->swapchainImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = initialLayout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    foeGfxVkRenderGraphResourceCreateInfo resourceCI{
        .sType = FOE_NULL_HANDLE,
        .pNext = nullptr,
        .pName = pResourceName,
        .isMutable = true,
        .pResourceData = &pJobResources->swapchainImage,
    };

    foeGfxVkRenderGraphResourceHandle newSwapchainResource;
    foeResultSet result =
        foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &newSwapchainResource);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
        return result;
    }

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

    result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();

        return result;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .provider = renderGraphJob,
        .resource = newSwapchainResource,
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
        foeGfxVkRenderGraphGetResourceData(swapchainResource.resource),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN);

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
                     uint32_t waitSemaphoreCount, VkSemaphore *pWaitSemaphores,
                     uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers,
                     uint32_t signalSemaphoreCount, VkSemaphore *pSignalSemaphores,
                     VkFence fence) -> foeResultSet {
        VkResult vkResult;

        // If we have a fence, then we need to signal it
        VkSemaphore extraSemaphore{VK_NULL_HANDLE};

        if (commandBufferCount > 0 || signalSemaphoreCount > 0 || fence != VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            vkResult = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                         &extraSemaphore);
            if (vkResult)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_VkSemaphore,
                                        (void *)extraSemaphore);

            std::vector<VkPipelineStageFlags> waitMask{waitSemaphoreCount,
                                                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};

            VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = waitSemaphoreCount,
                .pWaitSemaphores = pWaitSemaphores,
                .pWaitDstStageMask = waitMask.data(),
                .signalSemaphoreCount = 1U,
                .pSignalSemaphores = &extraSemaphore,
            };

            auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
            vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
            foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
            if (vkResult != VK_SUCCESS)
                std::abort();
        }

        // Now get on with presenting the image
        auto const *pSwapchainData =
            (foeGfxVkGraphSwapchainResource const *)foeGfxVkGraphFindStructure(
                foeGfxVkRenderGraphGetResourceData(swapchainResource.resource),
                RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN);

        VkResult presentRes{VK_SUCCESS};
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = (extraSemaphore != VK_NULL_HANDLE) ? 1U : waitSemaphoreCount,
            .pWaitSemaphores =
                (extraSemaphore != VK_NULL_HANDLE) ? &extraSemaphore : pWaitSemaphores,
            .swapchainCount = 1U,
            .pSwapchains = &pSwapchainData->swapchain,
            .pImageIndices = &pSwapchainData->index,
            .pResults = &presentRes,
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
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
    bool resourceReadOnly = false;
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