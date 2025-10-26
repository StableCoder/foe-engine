// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_SCENE_HPP
#define RENDER_SCENE_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/result.h>

struct foeSimulation;

foeResultSet renderSceneJob(foeGfxVkRenderGraph renderGraph,
                            char const *pJobName,
                            VkFence fence,
                            foeGfxVkRenderGraphResource colourRenderTarget,
                            uint32_t colourRenderTargetUpstreamJobCount,
                            foeGfxVkRenderGraphJob const *pColourRenderTargetUpstreamJobs,
                            VkImageLayout finalColourLayout,
                            foeGfxVkRenderGraphResource depthRenderTarget,
                            uint32_t depthRenderTargetUpstreamJobCount,
                            foeGfxVkRenderGraphJob const *pDepthRenderTargetUpstreamJobs,
                            VkImageLayout finalDepthLayout,
                            VkSampleCountFlags renderTargetSamples,
                            foeSimulation *pSimulation,
                            VkDescriptorSet cameraDescriptor,
                            uint32_t frameIndex,
                            foeGfxVkRenderGraphJob *pRenderGraphJob);

#endif // RENDER_SCENE_HPP