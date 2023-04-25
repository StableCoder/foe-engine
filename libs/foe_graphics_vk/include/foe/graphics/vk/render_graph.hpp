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
FOE_DEFINE_HANDLE(foeGfxVkRenderGraphResource)

typedef std::function<void()> foeGfxVkRenderGraphFn;

typedef std::function<foeResultSet(foeGfxSession,
                                   foeGfxDelayedCaller,
                                   uint32_t,
                                   VkSemaphore *,
                                   uint32_t,
                                   VkCommandBuffer *,
                                   uint32_t,
                                   VkSemaphore *,
                                   VkFence)>
    PFN_foeGfxVkRenderGraphCustomSubmit;

typedef std::function<foeResultSet(foeGfxSession, foeGfxDelayedCaller, VkCommandBuffer)>
    PFN_foeGfxVkRenderGraphFillCmdBuffer;

enum foeGfxVkRenderGraphStructureType {
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
};

enum foeGfxVkRenderGraphResourceMode {
    RENDER_GRAPH_RESOURCE_MODE_READ_WRITE = 0,
    RENDER_GRAPH_RESOURCE_MODE_READ_ONLY = 1,
};

struct foeGfxVkRenderGraphStructure {
    foeGfxVkRenderGraphStructureType sType;
    void *pNext;
};

FOE_GFX_EXPORT
foeGfxVkRenderGraphStructure const *foeGfxVkGraphFindStructure(
    foeGfxVkRenderGraphStructure const *pData, foeGfxVkRenderGraphStructureType sType);

FOE_GFX_EXPORT
foeResultSet foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph);

FOE_GFX_EXPORT
void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph);

struct foeGfxVkRenderGraphResourceCreateInfo {
    uint32_t sType;
    void *pNext;
    char const *pName;
    bool isMutable;
    void *pResourceData;
};

FOE_GFX_EXPORT
foeResultSet foeGfxVkRenderGraphCreateResource(
    foeGfxVkRenderGraph renderGraph,
    foeGfxVkRenderGraphResourceCreateInfo *pResoureCreateInfo,
    foeGfxVkRenderGraphResource *pResource);

FOE_GFX_EXPORT
char const *foeGfxVkRenderGraphGetResourceName(foeGfxVkRenderGraphResource resource);
FOE_GFX_EXPORT
bool foeGfxVkRenderGraphGetResourceIsMutable(foeGfxVkRenderGraphResource resource);
FOE_GFX_EXPORT
foeGfxVkRenderGraphStructure const *foeGfxVkRenderGraphGetResourceData(
    foeGfxVkRenderGraphResource resource);

struct foeGfxVkRenderGraphResourceState {
    uint32_t upstreamJobCount;
    foeGfxVkRenderGraphJob const *pUpstreamJobs;
    foeGfxVkRenderGraphResourceMode mode;
    foeGfxVkRenderGraphResource resource;
    foeGfxVkRenderGraphStructure const *pIncomingState;
    foeGfxVkRenderGraphStructure const *pOutgoingState;
};

struct foeGfxVkRenderGraphJobInfo {
    uint32_t resourceCount;
    foeGfxVkRenderGraphResourceState const *pResources;
    foeGfxVkRenderGraphFn freeDataFn;
    std::string_view name;
    bool required;
    VkQueueFlags queueFlags;
    void *pExtraSubmitInfo;
    uint32_t otherUpstreamJobCount;
    foeGfxVkRenderGraphJob *pOtherUpstreamJobs;
    uint32_t waitSemaphoreCount;
    VkSemaphore *pWaitSemaphores;
    uint32_t signalSemaphoreCount;
    VkSemaphore *pSignalSemaphores;
    VkFence fence;
};

FOE_GFX_EXPORT
foeResultSet foeGfxVkRenderGraphAddJob(foeGfxVkRenderGraph renderGraph,
                                       foeGfxVkRenderGraphJobInfo *pJobInfo,
                                       PFN_foeGfxVkRenderGraphCustomSubmit &&customSubmit,
                                       PFN_foeGfxVkRenderGraphFillCmdBuffer &&fillCmdBuf,
                                       foeGfxVkRenderGraphJob *pJob);

FOE_GFX_EXPORT
bool foeGfxVkRenderGraphJobToExecute(foeGfxVkRenderGraphJob job);

FOE_GFX_EXPORT
foeResultSet foeGfxVkRenderGraphCompile(foeGfxVkRenderGraph renderGraph);

FOE_GFX_EXPORT
foeResultSet foeGfxVkRenderGraphExecute(foeGfxVkRenderGraph renderGraph,
                                        foeGfxSession gfxSession,
                                        foeGfxDelayedCaller gfxDelayedDestructor);

#endif // FOE_GRAPHICS_VK_RENDER_GRAPH_HPP