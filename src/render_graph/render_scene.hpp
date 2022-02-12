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

#ifndef RENDER_SCENE_HPP
#define RENDER_SCENE_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/vk/render_graph.hpp>

#include <system_error>

struct foeSimulationState;

struct RenderSceneOutputResources {
    foeGfxVkRenderGraphResource colourRenderTarget;
    foeGfxVkRenderGraphResource depthRenderTarget;
};

std::error_code renderSceneJob(foeGfxVkRenderGraph renderGraph,
                               std::string_view name,
                               VkFence fence,
                               foeGfxVkRenderGraphResource colourRenderTarget,
                               VkImageLayout initialColourLayout,
                               VkImageLayout finalColourLayout,
                               foeGfxVkRenderGraphResource depthRenderTarget,
                               VkImageLayout initialDepthLayout,
                               VkImageLayout finalDepthLayout,
                               VkSampleCountFlags renderTargetSamples,
                               foeSimulationState *pSimulationState,
                               VkDescriptorSet cameraDescriptor,
                               RenderSceneOutputResources &outputResources);

#endif // RENDER_SCENE_HPP