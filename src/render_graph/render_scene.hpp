// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_SCENE_HPP
#define RENDER_SCENE_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/graphics/vk/render_graph.hpp>

struct foeSimulation;

struct RenderSceneOutputResources {
    foeGfxVkRenderGraphResource colourRenderTarget;
    foeGfxVkRenderGraphResource depthRenderTarget;
};

foeResultSet renderSceneJob(foeGfxVkRenderGraph renderGraph,
                            std::string_view name,
                            VkFence fence,
                            foeGfxVkRenderGraphResource colourRenderTarget,
                            VkImageLayout finalColourLayout,
                            foeGfxVkRenderGraphResource depthRenderTarget,
                            VkImageLayout finalDepthLayout,
                            VkSampleCountFlags renderTargetSamples,
                            foeSimulation *pSimulation,
                            VkDescriptorSet cameraDescriptor,
                            RenderSceneOutputResources &outputResources);

#endif // RENDER_SCENE_HPP