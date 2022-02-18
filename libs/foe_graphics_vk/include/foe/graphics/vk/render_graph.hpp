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
FOE_DEFINE_HANDLE(foeGfxVkRenderGraphJob)

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
    foeGfxVkRenderGraphStructure const *pResourceData;
    foeGfxVkRenderGraphStructure const *pResourceState;
};

FOE_GFX_EXPORT auto foeGfxVkCreateRenderGraph(foeGfxVkRenderGraph *pRenderGraph) -> std::error_code;

FOE_GFX_EXPORT void foeGfxVkDestroyRenderGraph(foeGfxVkRenderGraph renderGraph);

FOE_GFX_EXPORT auto foeGfxVkRenderGraphAddJob(
    foeGfxVkRenderGraph renderGraph,
    uint32_t resourcesCount,
    foeGfxVkRenderGraphResource const *pResourcesIn,
    bool const *pResourcesInReadOnly,
    foeGfxVkRenderGraphFn freeDataFn,
    std::string_view name,
    bool required,
    std::function<std::error_code(foeGfxSession,
                                  foeGfxDelayedDestructor,
                                  std::vector<VkSemaphore> const &,
                                  std::vector<VkSemaphore> const &,
                                  std::function<void(std::function<void()>)>)> &&jobFn,
    foeGfxVkRenderGraphJob *pJob) -> std::error_code;

FOE_GFX_EXPORT auto foeGfxVkExecuteRenderGraph(foeGfxVkRenderGraph renderGraph,
                                               foeGfxSession gfxSession,
                                               foeGfxDelayedDestructor gfxDelayedDestructor)
    -> std::error_code;

FOE_GFX_EXPORT void foeGfxVkExecuteRenderGraphCpuJobs(foeGfxVkRenderGraph renderGraph);

#endif // FOE_GRAPHICS_VK_RENDER_GRAPH_HPP