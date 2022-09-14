// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph/job/import_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.h>

#include "../../vk_result.h"

foeResultSet foeGfxVkImportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                          std::string_view name,
                                          VkFence fence,
                                          std::string_view imageName,
                                          VkImage image,
                                          VkImageView view,
                                          VkFormat format,
                                          VkExtent2D extent,
                                          VkImageLayout layout,
                                          bool isMutable,
                                          std::vector<VkSemaphore> waitSemaphores,
                                          foeGfxVkRenderGraphResource *pResourcesOut) {
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResultSet {
        auto realWaitSemaphores = waitSemaphores;
        std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitMasks.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        VkResult vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return vk_to_foeResult(vkResult);
    };

    // Resource management
    auto *pImportedImage = new foeGfxVkGraphImageResource;
    *pImportedImage = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = nullptr,
        .name = std::string{imageName},
        .image = image,
        .view = view,
        .format = format,
        .extent = extent,
        .isMutable = isMutable,
    };

    auto *pImageState = new foeGfxVkGraphImageState;
    *pImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = layout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        delete pImportedImage;
        delete pImageState;
    };

    // Add job to graph
    foeGfxVkRenderGraphJob renderGraphJob;

    foeResultSet result = foeGfxVkRenderGraphAddJob(renderGraph, 0, nullptr, nullptr, freeDataFn,
                                                    name, false, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {
        // Outgoing resources
        *pResourcesOut = foeGfxVkRenderGraphResource{
            .provider = renderGraphJob,
            .pResourceData = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImportedImage),
            .pResourceState = reinterpret_cast<foeGfxVkRenderGraphStructure const *>(pImageState),
        };
    }

    return result;
}