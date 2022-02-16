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

#include <foe/graphics/vk/render_graph/job/present_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include "../../error_code.hpp"

struct foeGfxVkGraphSwapchainResource {
    foeGfxVkGraphStructureType sType;
    void *pNext;
    VkSwapchainKHR swapchain;
    uint32_t index;
};

auto foeGfxVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                           std::string_view name,
                                           VkFence fence,
                                           std::string_view resourceName,
                                           VkSwapchainKHR swapchain,
                                           uint32_t index,
                                           VkImage image,
                                           VkImageView view,
                                           VkFormat format,
                                           VkExtent2D extent,
                                           VkImageLayout initialLayout,
                                           VkSemaphore waitSemaphore,
                                           foeGfxVkRenderGraphResource *pResourcesOut)
    -> std::error_code {
    std::error_code errC;

    auto pJob = new RenderGraphJob;
    pJob->name = name;
    pJob->required = false;
    pJob->executeFn =
        [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
            std::vector<VkSemaphore> const &, std::vector<VkSemaphore> const &signalSemaphores,
            std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
        std::error_code errC;

        VkPipelineStageFlags waitMask{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSemaphore waitSemaphores{waitSemaphore};

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSemaphores,
            .pWaitDstStageMask = &waitMask,
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return errC;
    };

    // Resource management
    auto *pSwapchainImage = new foeGfxVkGraphSwapchainResource;
    *pSwapchainImage = foeGfxVkGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
        .index = index,
    };

    auto *pImage = new foeGfxVkGraphImageResource;
    *pImage = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = pSwapchainImage,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    auto *pImageState = new foeGfxVkGraphImageState;
    *pImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = initialLayout,
    };

    DeleteResourceDataCall deleteCalls[2] = {
        {
            .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
                auto *pImage = reinterpret_cast<foeGfxVkGraphImageResource *>(pResource);
                auto *pSwapchain =
                    reinterpret_cast<foeGfxVkGraphSwapchainResource *>(pImage->pNext);

                delete pSwapchain;
                delete pImage;
            },
            .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pImage),
        },
        {
            .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
                delete reinterpret_cast<foeGfxVkGraphImageState *>(pResource);
            },
            .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pImageState),
        },
    };

    // Add job to graph
    errC = foeGfxVkRenderGraphAddJob(renderGraph, pJob, 0, nullptr, nullptr, 2, deleteCalls);
    if (errC) {
        for (auto const &it : deleteCalls) {
            it.deleteFn(it.pResource);
        }

        return errC;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .pProvider = pJob,
        .pResourceData = reinterpret_cast<foeGfxVkGraphStructure *>(pImage),
        .pResourceState = reinterpret_cast<foeGfxVkGraphStructure *>(pImageState),
    };

    return FOE_GRAPHICS_VK_SUCCESS;
}

auto foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                            std::string_view name,
                                            VkFence fence,
                                            foeGfxVkRenderGraphResource swapchainResource)
    -> std::error_code {
    // Check that the given resource is the correct type
    auto *pSwapchainData = (foeGfxVkGraphSwapchainResource *)foeGfxVkGraphFindStructure(
        swapchainResource.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN);

    if (pSwapchainData == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN;

    // Check that the image state is correct
    auto *pImageState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        swapchainResource.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pImageState == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE;
    if (pImageState->layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        std::abort();

    auto pJob = new RenderGraphJob;
    pJob->name = name;
    // The image is being presented as output, and thus needs to happen
    pJob->required = true;
    pJob->executeFn =
        [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
            std::vector<VkSemaphore> const &waitSemaphores,
            std::vector<VkSemaphore> const &signalSemaphores,
            std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
        std::error_code errC;

        // If we have a fence, then we need to signal it
        VkPipelineStageFlags waitMask{VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
        VkSemaphore signalSemaphore{VK_NULL_HANDLE};

        { // Create temporary semaphore
            VkSemaphoreCreateInfo semaphoreCI{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            errC = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                     &signalSemaphore);
            if (errC)
                return errC;

            foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                vkDestroySemaphore(foeGfxVkGetDevice(session), signalSemaphore, nullptr);
            });
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
        errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

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
        errC = vkQueuePresentKHR(queue, &presentInfo);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        if (errC == VK_ERROR_OUT_OF_DATE_KHR || errC == VK_SUBOPTIMAL_KHR) {
            // The associated window has been resized, will be fixed for the next frame
            errC = VK_SUCCESS;
        } else if (errC != VK_SUCCESS) {
            return errC;
        }

        if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR) {
            // The associated window has been resized, will be fixed for the next frame
            errC = VK_SUCCESS;
        } else if (presentRes != VK_SUCCESS) {
            return presentRes;
        }

        return errC;
    };

    // Add job to graph
    bool const resourceReadOnly = false;

    return foeGfxVkRenderGraphAddJob(renderGraph, pJob, 1, &swapchainResource, &resourceReadOnly, 0,
                                     nullptr);
}