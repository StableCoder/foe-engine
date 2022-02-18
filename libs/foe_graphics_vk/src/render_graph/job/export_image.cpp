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

#include <foe/graphics/vk/render_graph/job/export_image.hpp>

#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include "../../error_code.hpp"

auto foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                  std::string_view name,
                                  VkFence fence,
                                  foeGfxVkRenderGraphResource resource,
                                  VkImageLayout requiredLayout,
                                  std::vector<VkSemaphore> signalSemaphores) -> std::error_code {
    // Check that this is an image resource
    auto const *pImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        resource.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pImageData == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE;

    // Check that this is in the correct/desired state
    auto const *pImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        resource.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pImageState == nullptr)
        return FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE;
    if (pImageState->layout != requiredLayout)
        std::abort();

    // Proceed with job creation
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
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
        std::error_code errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return errC;
    };

    // Add job to graph
    bool const readOnly = true;
    foeGfxVkRenderGraphJob renderGraphJob;

    return foeGfxVkRenderGraphAddJob(renderGraph, 1, &resource, &readOnly, nullptr, name, true,
                                     std::move(jobFn), &renderGraphJob);
}