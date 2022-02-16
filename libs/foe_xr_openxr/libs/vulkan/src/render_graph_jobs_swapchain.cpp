/*
    Copyright (C) 2022 George Cave.

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

#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/xr/openxr/error_code.hpp>
#include <vk_error_code.hpp>

#include "error_code.hpp"
#include "log.hpp"

struct foeOpenXrRenderGraphSwapchainResource {
    foeGfxVkGraphStructureType sType;
    void *pNext;
    XrSwapchain swapchain;
};

auto foeOpenXrVkImportSwapchainImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                              std::string_view name,
                                              VkFence fence,
                                              std::string_view resourceName,
                                              XrSwapchain swapchain,
                                              VkImage image,
                                              VkImageView view,
                                              VkFormat format,
                                              VkExtent2D extent,
                                              VkImageLayout layout,
                                              foeGfxVkRenderGraphResource *pResourcesOut)
    -> std::error_code {
    std::error_code errC;

    // Add the job that waits on the timeline semaphore and signals any dependent jobs to start
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
        std::error_code errC;

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
        errC = vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                 &timelineSemaphore);
        if (errC)
            return errC;

        foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
            vkDestroySemaphore(foeGfxVkGetDevice(session), timelineSemaphore, nullptr);
        });

        addCpuFnFn([=]() {
            XrSwapchainImageWaitInfo waitInfo{
                .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
                .timeout = 10000, // In nanoseconds (0.01 ms)
            };

            XrResult xrRes{XR_TIMEOUT_EXPIRED};

            do {
                xrRes = xrWaitSwapchainImage(swapchain, &waitInfo);
            } while (xrRes == XR_TIMEOUT_EXPIRED);

            if (xrRes != XR_SUCCESS && xrRes != XR_SESSION_LOSS_PENDING) {
                std::error_code errC = xrRes;
                FOE_LOG(foeOpenXrVk, Error, "xrWaitSwapchainImage failed: {}", errC.message());
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

            std::error_code errC = xrReleaseSwapchainImage(swapchain, &releaseInfo);
            if (errC) {
                FOE_LOG(foeOpenXrVk, Fatal, "xrReleaseSwapchainImage error: {}", errC.message())
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
        errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return errC;
    };

    // Resource management
    auto *pSwapchain = new foeOpenXrRenderGraphSwapchainResource;
    *pSwapchain = foeOpenXrRenderGraphSwapchainResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
        .pNext = nullptr,
        .swapchain = swapchain,
    };

    auto *pImage = new foeGfxVkGraphImageResource;
    *pImage = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = pSwapchain,
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = true,
    };

    auto *pImageState = new foeGfxVkGraphImageState;
    *pImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };

    DeleteResourceDataCall deleteCalls[2]{
        {
            .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
                auto *pImage = reinterpret_cast<foeGfxVkGraphImageResource *>(pResource);
                auto *pSwapchain =
                    reinterpret_cast<foeOpenXrRenderGraphSwapchainResource *>(pImage->pNext);

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
    foeGfxVkRenderGraphJob renderGraphJob;

    errC = foeGfxVkRenderGraphAddJob(renderGraph, 0, nullptr, nullptr, 2, deleteCalls, name, true,
                                     std::move(jobFn), &renderGraphJob);
    if (errC) {
        // If we couldn't add it to the render graph, delete all heap data now
        for (auto const &it : deleteCalls) {
            it.deleteFn(it.pResource);
        }

        return errC;
    }

    // Outgoing resources
    *pResourcesOut = foeGfxVkRenderGraphResource{
        .provider = renderGraphJob,
        .pResourceData = reinterpret_cast<foeGfxVkGraphStructure *>(pImage),
        .pResourceState = reinterpret_cast<foeGfxVkGraphStructure *>(pImageState),
    };

    return FOE_OPENXR_VK_SUCCESS;
}
