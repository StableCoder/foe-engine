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

struct RenderGraphVkSwapchain {
    RenderGraphResourceStructureType sType;
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
                                           VkSemaphore waitSemaphore) -> RenderGraphResource {
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

    /// @todo Replace with C-style altogether value
    auto *pSwapchainImage = new RenderGraphVkSwapchain;
    *pSwapchainImage = RenderGraphVkSwapchain{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
        .index = index,
    };

    auto *pImage = new RenderGraphResourceImage;
    *pImage = RenderGraphResourceImage{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = pSwapchainImage,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    RenderGraphResource swapchainImage{
        .pProvider = pJob,
        .pResourceData = reinterpret_cast<RenderGraphResourceBase *>(pImage),
    };

    DeleteResourceDataCall deleteResCall{
        .deleteFn = [](RenderGraphResourceBase *pResource) -> void {
            auto *pImage = reinterpret_cast<RenderGraphResourceImage *>(pResource);
            auto *pSwapchain = reinterpret_cast<RenderGraphVkSwapchain *>(pImage->pNext);

            delete pSwapchain;
            delete pImage;
        },
        .pResource = reinterpret_cast<RenderGraphResourceBase *>(pImage),
    };

    foeGfxVkRenderGraphAddJob(renderGraph, pJob, 0, nullptr, nullptr, 1, &deleteResCall, nullptr);

    return swapchainImage;
}

void foeGfxVkPresentSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                            std::string_view name,
                                            RenderGraphResource swapchainResource) {
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

        RenderGraphVkSwapchain *pSwapchainResource =
            reinterpret_cast<RenderGraphVkSwapchain *>(swapchainResource.pResourceData->pNext);

        VkResult presentRes{VK_SUCCESS};
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .swapchainCount = 1U,
            .pSwapchains = &pSwapchainResource->swapchain,
            .pImageIndices = &pSwapchainResource->index,
            .pResults = &presentRes,
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
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

    RenderGraphResource resourceOut;
    bool const resourceReadOnly = false;

    foeGfxVkRenderGraphAddJob(renderGraph, pJob, 1, &swapchainResource, &resourceReadOnly, 0,
                              nullptr, &resourceOut);
}