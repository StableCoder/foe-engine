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

#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

void foeGfxVkExportImageRenderJob(foeGfxVkRenderGraph renderGraph,
                                  std::string_view name,
                                  VkFence fence,
                                  foeGfxVkRenderGraphResource resource,
                                  VkImageLayout layout,
                                  std::vector<VkSemaphore> signalSemaphores) {
    auto pJob = new RenderGraphJob;
    *pJob = RenderGraphJob{
        .name = std::string{name},
        // This image is being exported/output, and thus needs to happen
        .required = true,
        .executeFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
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
        },
    };

    bool const readOnly = true;
    foeGfxVkRenderGraphResource usedResource;

    foeGfxVkRenderGraphAddJob(renderGraph, pJob, 1, &resource, &readOnly, 0, nullptr,
                              &usedResource);
}