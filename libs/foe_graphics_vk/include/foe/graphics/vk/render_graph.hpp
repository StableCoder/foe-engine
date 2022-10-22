// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RENDER_GRAPH_HPP
#define FOE_GRAPHICS_VK_RENDER_GRAPH_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <string>

FOE_DEFINE_HANDLE(foeGfxVkRenderGraph)
FOE_DEFINE_HANDLE(foeGfxVkRenderGraphJob)
FOE_DEFINE_HANDLE(foeGfxVkRenderGraphResourceHandle)

typedef std::function<void()> foeGfxVkRenderGraphFn;

enum foeGfxVkRenderGraphStructureType {
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
};

struct foeGfxVkRenderGraphStructure {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
};

FOE_GFX_EXPORT foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindStructure(
    foeGfxVkRenderGraphStructure const *pData, foeGfxVkRenderGraphStructureType sType);

struct foeGfxVkRenderGraphResource {
    foeGfxVkRenderGraphJob provider;
    foeGfxVkRenderGraphResourceHandle resource;
    foeGfxVkRenderGraphStructure const *pResourceState;
};

FOE_GFX_EXPORT foeResultSet foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph);

FOE_GFX_EXPORT void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph);

struct foeGfxVkRenderGraphResourceCreateInfo {
    uint32_t sType;
    void *pNext;
    char const *pName;
    bool isMutable;
    void *pResourceData;
};

FOE_GFX_EXPORT foeResultSet
foeGfxVkRenderGraphCreateResource(foeGfxVkRenderGraph renderGraph,
                                  foeGfxVkRenderGraphResourceCreateInfo *pResoureCreateInfo,
                                  foeGfxVkRenderGraphResourceHandle *pResource);

FOE_GFX_EXPORT char const *foeGfxVkRenderGraphGetResourceName(
    foeGfxVkRenderGraphResourceHandle resource);
FOE_GFX_EXPORT bool foeGfxVkRenderGraphGetResourceIsMutable(
    foeGfxVkRenderGraphResourceHandle resource);
FOE_GFX_EXPORT foeGfxVkRenderGraphStructure const *foeGfxVkRenderGraphGetResourceData(
    foeGfxVkRenderGraphResourceHandle resource);

struct foeGfxVkRenderGraphResourceState {
    foeGfxVkRenderGraphJob upstreamJob;
    bool readOnly;
    foeGfxVkRenderGraphResourceHandle resource;
    foeGfxVkRenderGraphStructure const *pIncomingState;
    foeGfxVkRenderGraphStructure const *pOutgoingState;
};

struct foeGfxVkRenderGraphJobInfo {
    uint32_t resourceCount;
    foeGfxVkRenderGraphResource *pResourcesIn;
    bool *pResourcesInReadOnly;
    foeGfxVkRenderGraphFn freeDataFn;
    std::string_view name;
    bool required;
    VkQueueFlags queueFlags;
    void *pExtraSubmitInfo;
    uint32_t waitSemaphoreCount;
    VkSemaphore *pWaitSemaphores;
    uint32_t signalSemaphoreCount;
    VkSemaphore *pSignalSemaphores;
    VkFence fence;
};

FOE_GFX_EXPORT foeResultSet foeGfxVkRenderGraphAddJob(
    foeGfxVkRenderGraph renderGraph,
    foeGfxVkRenderGraphJobInfo *pJobInfo,
    std::function<foeResultSet(foeGfxSession,
                               foeGfxDelayedCaller,
                               std::vector<VkSemaphore> const &,
                               std::vector<VkSemaphore> const &)> &&customJob,
    std::function<foeResultSet(foeGfxSession, foeGfxDelayedCaller, VkCommandBuffer)>
        &&fillCommandBufferFn,
    foeGfxVkRenderGraphJob *pJob);

FOE_GFX_EXPORT foeResultSet foeGfxVkExecuteRenderGraph(foeGfxVkRenderGraph renderGraph,
                                                       foeGfxSession gfxSession,
                                                       foeGfxDelayedCaller gfxDelayedDestructor);

#endif // FOE_GRAPHICS_VK_RENDER_GRAPH_HPP