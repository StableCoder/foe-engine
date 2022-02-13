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

#ifndef FOE_GRAPHICS_VK_RENDER_GRAPH_HPP
#define FOE_GRAPHICS_VK_RENDER_GRAPH_HPP

#include <foe/graphics/delayed_destructor.hpp>
#include <foe/graphics/export.h>
#include <foe/graphics/session.hpp>
#include <foe/handle.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <string>
#include <system_error>

FOE_DEFINE_HANDLE(foeGfxVkRenderGraph)

enum foeGfxVkGraphStructureType {
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_VK_SWAPCHAIN,
    RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_XR_SWAPCHAIN,
};

struct foeGfxVkGraphStructure {
    foeGfxVkGraphStructureType sType;
    void *pNext;
};

FOE_GFX_EXPORT foeGfxVkGraphStructure *foeGfxVkGraphFindStructure(
    foeGfxVkGraphStructure const *pData, foeGfxVkGraphStructureType sType);

struct RenderGraphJob {
    /// Name of the job for debugging, mapping and logging purposes
    std::string name;
    /// This job needs to be run as part of the render graph's execution, typically as the job is
    /// outputting something, but other reasons exist.
    bool required{false};
    bool processed{false};
    std::function<std::error_code(foeGfxSession,
                                  foeGfxDelayedDestructor,
                                  std::vector<VkSemaphore> const &,
                                  std::vector<VkSemaphore> const &,
                                  std::function<void(std::function<void()>)>)>
        executeFn;
};

struct foeGfxVkRenderGraphResource {
    RenderGraphJob *pProvider;
    foeGfxVkGraphStructure *pResourceData;
    foeGfxVkGraphStructure *pResourceState;
};

struct DeleteResourceDataCall {
    std::function<void(foeGfxVkGraphStructure *)> deleteFn;
    foeGfxVkGraphStructure *pResource;
};

FOE_GFX_EXPORT auto foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph) -> std::error_code;

FOE_GFX_EXPORT void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph);

FOE_GFX_EXPORT auto foeGfxVkRenderGraphAddJob(foeGfxVkRenderGraph renderGraph,
                                              RenderGraphJob *pJob,
                                              uint32_t resourcesCount,
                                              foeGfxVkRenderGraphResource const *pResourcesIn,
                                              bool const *pResourcesInReadOnly,
                                              uint32_t deleteResourceCallsCount,
                                              DeleteResourceDataCall *pDeleteResourceCalls,
                                              foeGfxVkRenderGraphResource *pResourcesOut)
    -> std::error_code;

FOE_GFX_EXPORT auto foeGfxVkExecuteRenderGraph(foeGfxVkRenderGraph renderGraph,
                                               foeGfxSession gfxSession,
                                               foeGfxDelayedDestructor gfxDelayedDestructor)
    -> std::error_code;

FOE_GFX_EXPORT void foeGfxVkExecuteRenderGraphCpuJobs(foeGfxVkRenderGraph renderGraph);

#endif // FOE_GRAPHICS_VK_RENDER_GRAPH_HPP