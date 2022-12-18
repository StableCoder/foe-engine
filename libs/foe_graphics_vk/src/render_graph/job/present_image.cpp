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
                                                   foeGfxVkRenderGraphResource *pSwapchainResource,
                                                   foeGfxVkRenderGraphJob *pRenderGraphJob) {
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
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    foeGfxVkRenderGraphResourceCreateInfo resourceCI{
        .sType = FOE_NULL_HANDLE,
        .pNext = nullptr,
        .pName = pResourceName,
        .isMutable = true,
        .pResourceData = &pJobResources->swapchainImage,
    };

    foeGfxVkRenderGraphResource newSwapchainResource = FOE_NULL_HANDLE;
    foeResultSet result =
        foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &newSwapchainResource);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
        return result;
    }

    // Add job to graph
    foeGfxVkRenderGraphResourceState resourceState = foeGfxVkRenderGraphResourceState{
        .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
        .resource = newSwapchainResource,
        .pIncomingState = nullptr,
        .pOutgoingState = (foeGfxVkRenderGraphStructure *)&pJobResources->swapchainImageState,
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResources = &resourceState,
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,
        .fence = fence,
    };

    result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pRenderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();

        return result;
    } else {
        // Outgoing resources
        *pSwapchainResource = newSwapchainResource;
    }

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

namespace {

struct ExtraJobData {
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<uint32_t> swapchainImageIndexes;
};

void destroy_VkSemaphore(VkSemaphore semaphore, foeGfxSession session) {
    vkDestroySemaphore(foeGfxVkGetDevice(session), semaphore, nullptr);
}

} // namespace

foeResultSet foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                                    char const *pJobName,
                                                    VkFence fence,
                                                    uint32_t presentInfoCount,
                                                    foeGfxVkSwapchainPresentInfo *pPresentInfos) {
    ExtraJobData *pExtraJobData = new ExtraJobData;

    for (uint32_t i = 0; i < presentInfoCount; ++i) {
        foeGfxVkGraphSwapchainResource const *pSwapchainData =
            (foeGfxVkGraphSwapchainResource const *)foeGfxVkGraphFindStructure(
                foeGfxVkRenderGraphGetResourceData(pPresentInfos[i].swapchainResource),
                RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN);

        if (pSwapchainData == nullptr)
            return to_foeResult(
                FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN);

        pExtraJobData->swapchains.emplace_back(pSwapchainData->swapchain);
        pExtraJobData->swapchainImageIndexes.emplace_back(pSwapchainData->index);
    }

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
        std::vector<VkResult> presentResults{pExtraJobData->swapchains.size(), VK_SUCCESS};
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = (extraSemaphore != VK_NULL_HANDLE) ? 1U : waitSemaphoreCount,
            .pWaitSemaphores =
                (extraSemaphore != VK_NULL_HANDLE) ? &extraSemaphore : pWaitSemaphores,
            .swapchainCount = (uint32_t)pExtraJobData->swapchains.size(),
            .pSwapchains = pExtraJobData->swapchains.data(),
            .pImageIndices = pExtraJobData->swapchainImageIndexes.data(),
            .pResults = presentResults.data(),
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

        for (auto &result : presentResults) {
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                // The associated window has been resized, will be fixed for the next frame
                vkResult = VK_SUCCESS;
            } else if (result != VK_SUCCESS) {
                return vk_to_foeResult(result);
            }
        }

        return vk_to_foeResult(vkResult);
    };

    // Resource Management
    auto *pSwapchainResourceState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        delete pExtraJobData;
        delete pSwapchainResourceState;
    };

    // Add job to graph
    std::vector<foeGfxVkRenderGraphResourceState> resourceStates;

    for (uint32_t i = 0; i < presentInfoCount; ++i) {
        resourceStates.emplace_back(foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = pPresentInfos[i].upstreamJobCount,
            .pUpstreamJobs = pPresentInfos[i].pUpstreamJobs,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = pPresentInfos[i].swapchainResource,
            .pIncomingState = (foeGfxVkRenderGraphStructure *)pSwapchainResourceState,
            .pOutgoingState = nullptr,
        });
    }

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = (uint32_t)resourceStates.size(),
        .pResources = resourceStates.data(),
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = true,
        .fence = fence,
    };

    foeGfxVkRenderGraphJob renderGraphJob;
    return foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, std::move(jobFn), {}, &renderGraphJob);
}