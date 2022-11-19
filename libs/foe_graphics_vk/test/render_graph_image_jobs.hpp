// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/result.h>

static foeResultSet createImageJob(foeGfxVkRenderGraph renderGraph,
                                   char const *pJobName,
                                   bool requiredJob,
                                   char const *pImageName,
                                   VkImageLayout initialLayout,
                                   bool mutableImage,
                                   foeGfxVkRenderGraphResource *pImageResource,
                                   foeGfxVkRenderGraphJob *pCreateJob) {
    struct ImportImageJobResources {
        foeGfxVkGraphImageResource imageResource;
        foeGfxVkGraphImageState imageState;
    };

    ImportImageJobResources *pJobResources = new (std::nothrow) ImportImageJobResources;
    if (pJobResources == nullptr)
        return foeResultSet{.value = FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY};

    pJobResources->imageResource = foeGfxVkGraphImageResource{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
        .pNext = nullptr,
        .image = VK_NULL_HANDLE,
        .view = VK_NULL_HANDLE,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = VkExtent2D{.width = 512, .height = 512},
    };

    pJobResources->imageState = foeGfxVkGraphImageState{
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
        .pName = pImageName,
        .isMutable = mutableImage,
        .pResourceData = &pJobResources->imageResource,
    };

    foeGfxVkRenderGraphResource newImageResource = FOE_NULL_HANDLE;
    foeResultSet result =
        foeGfxVkRenderGraphCreateResource(renderGraph, &resourceCI, &newImageResource);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
        return result;
    }

    // Add job to graph
    foeGfxVkRenderGraphResourceState resourceState = foeGfxVkRenderGraphResourceState{
        .mode = (mutableImage) ? RENDER_GRAPH_RESOURCE_MODE_READ_WRITE
                               : RENDER_GRAPH_RESOURCE_MODE_READ_ONLY,
        .resource = newImageResource,
        .pIncomingState = nullptr,
        .pOutgoingState = (foeGfxVkRenderGraphStructure *)&pJobResources->imageState,
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResources = &resourceState,
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = requiredJob,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .fence = VK_NULL_HANDLE,
    };

    result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pCreateJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    } else {
        *pImageResource = newImageResource;
    }

    return result;
}

static foeResultSet singleImageResourceJob(foeGfxVkRenderGraph renderGraph,
                                           char const *pJobName,
                                           bool requiredJob,
                                           foeGfxVkRenderGraphResource image,
                                           VkImageLayout incomingLayout,
                                           VkImageLayout outgoingLayout,
                                           foeGfxVkRenderGraphResourceMode mode,
                                           uint32_t imageUpstreamJobCount,
                                           foeGfxVkRenderGraphJob *pImageUpstreamJobs,
                                           foeGfxVkRenderGraphJob *pJob) {
    auto *pImageState = new foeGfxVkGraphImageState[2];

    pImageState[0] = {
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = incomingLayout,
    };
    pImageState[1] = {
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = outgoingLayout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete[] pImageState; };

    // Add job to graph
    foeGfxVkRenderGraphResourceState resourceState = foeGfxVkRenderGraphResourceState{
        .upstreamJobCount = imageUpstreamJobCount,
        .pUpstreamJobs = pImageUpstreamJobs,
        .mode = mode,
        .resource = image,
        .pIncomingState = (foeGfxVkRenderGraphStructure *)&pImageState[0],
        .pOutgoingState = (foeGfxVkRenderGraphStructure *)&pImageState[1],
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = 1,
        .pResources = &resourceState,
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = requiredJob,
        .fence = VK_NULL_HANDLE,
    };

    foeResultSet result = foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, {}, pJob);
    if (result.value != FOE_SUCCESS) {
        freeDataFn();
    }

    return result;
}