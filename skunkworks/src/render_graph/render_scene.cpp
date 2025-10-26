// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_scene.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/vk/mesh.h>
#include <foe/graphics/vk/pipeline_pool.h>
#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/render_pass_pool.h>
#include <foe/graphics/vk/session.h>
#include <foe/position/component/3d.hpp>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/simulation.hpp>
#include <vulkan/vulkan_core.h>

#include "../log.hpp"
#include "../result.h"
#include "../simulation/render_system.hpp"
#include "../vk_result.h"

namespace {

auto renderCall(RenderDataSet const *pDataSet,
                VkDescriptorSet positionDescriptorSet,
                VkDescriptorSet armatureDescriptorSet,
                foeGfxSession gfxSession,
                VkCommandBuffer commandBuffer,
                VkSampleCountFlags samples,
                VkRenderPass renderPass,
                VkDescriptorSet cameraDescriptor) -> bool {
    VkDescriptorSet const dummyDescriptorSet = foeGfxVkGetDummySet(gfxSession);

    foeResource vertexDescriptor{pDataSet->resources.vertexDescriptor};
    foeResource material{pDataSet->resources.material};
    foeResource mesh{pDataSet->resources.mesh};

    bool boned{false};
    if (pDataSet->resources.bonedVertexDescriptor != FOE_NULL_HANDLE &&
        armatureDescriptorSet != VK_NULL_HANDLE) {
        boned = true;

        vertexDescriptor = pDataSet->resources.bonedVertexDescriptor;
    }

    // Get Resource Data
    foeVertexDescriptor const *pVertexDescriptor =
        (foeVertexDescriptor const *)foeResourceGetTypeData(
            vertexDescriptor, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR);
    foeMaterial const *pMaterial = (foeMaterial const *)foeResourceGetTypeData(
        material, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL);
    foeMesh const *pMesh =
        (foeMesh const *)foeResourceGetTypeData(mesh, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH);

    // Retrieve the pipeline
    auto *pGfxVertexDescriptor = &pVertexDescriptor->vertexDescriptor;
    VkPipelineLayout layout;
    uint32_t descriptorSetLayoutCount;
    VkPipeline pipeline;

    auto pipelinePool = foeGfxVkGetPipelinePool(gfxSession);
    auto result = foeGfxVkGetPipeline(pipelinePool,
                                      const_cast<foeGfxVkVertexDescriptor *>(pGfxVertexDescriptor),
                                      pMaterial->pGfxFragDescriptor, renderPass, 0, samples,
                                      &layout, &descriptorSetLayoutCount, &pipeline);
    if (result.value != FOE_SUCCESS)
        std::abort();

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            &cameraDescriptor, 0, nullptr);

    foeGfxVkBindMesh(pMesh->gfxData, commandBuffer, boned);

    auto vertSetLayouts = foeGfxVkGetVertexDescriptorBuiltinSetLayouts(pGfxVertexDescriptor);
    if (vertSetLayouts & FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX) {
        // Bind the object's position *if* the descriptor supports it
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &positionDescriptorSet, 0, nullptr);
    } else {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &dummyDescriptorSet, 0, nullptr);
    }
    if (boned) {
        // If we have bone information, bind that too
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
                                &armatureDescriptorSet, 0, nullptr);
    }
    // Bind the fragment descriptor set *if* it exists?
    if (auto set = pMaterial->materialDescriptorSet; set != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                foeDescriptorSetLayoutIndex::FragmentShader, 1, &set, 0, nullptr);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDrawIndexed(commandBuffer, foeGfxGetMeshIndices(pMesh->gfxData), 1, 0, 0, 0);

    return true;
}

void destroy_VkFramebuffer(VkFramebuffer framebuffer, foeGfxSession session) {
    vkDestroyFramebuffer(foeGfxVkGetDevice(session), framebuffer, nullptr);
}

} // namespace

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
                            foeGfxVkRenderGraphJob *pRenderGraphJob) {
    // Make sure the resources passed in are images, and are mutable
    auto const *pColourImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(colourRenderTarget),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDepthImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        foeGfxVkRenderGraphGetResourceData(depthRenderTarget),
        RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pColourImageData == nullptr)
        return to_foeResult(FOE_SKUNKWORKS_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE);
    if (!foeGfxVkRenderGraphGetResourceIsMutable(colourRenderTarget))
        return to_foeResult(FOE_SKUNKWORKS_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE);

    if (pDepthImageData == nullptr)
        return to_foeResult(FOE_SKUNKWORKS_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE);
    if (!foeGfxVkRenderGraphGetResourceIsMutable(depthRenderTarget))
        return to_foeResult(FOE_SKUNKWORKS_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE);

    // Proceed with the job
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     VkCommandBuffer commandBuffer) -> foeResultSet {
        VkResult vkResult;

        foeGfxVkRenderPassPool renderPassPool = foeGfxVkGetRenderPassPool(gfxSession);

        std::array<VkAttachmentDescription, 2> attachmentDescriptions{
            VkAttachmentDescription{
                .format = pColourImageData->format,
                .samples = static_cast<VkSampleCountFlagBits>(renderTargetSamples),
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .finalLayout = finalColourLayout,
            },
            VkAttachmentDescription{
                .format = pDepthImageData->format,
                .samples = static_cast<VkSampleCountFlagBits>(renderTargetSamples),
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .finalLayout = finalDepthLayout,
            }};

        VkRenderPass renderPass = foeGfxVkGetRenderPass(
            renderPassPool, attachmentDescriptions.size(), attachmentDescriptions.data());

        VkFramebuffer framebuffer;

        { // Create Framebuffer
            std::array<VkImageView, 2> renderTargetViews = {pColourImageData->view,
                                                            pDepthImageData->view};
            VkFramebufferCreateInfo framebufferCI{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,
                .attachmentCount = renderTargetViews.size(),
                .pAttachments = renderTargetViews.data(),
                .width = pColourImageData->extent.width,
                .height = pColourImageData->extent.height,
                .layers = 1,
            };
            vkResult = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr,
                                           &framebuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_VkFramebuffer,
                                        (void *)framebuffer);
        }

        { // Setup common render viewport data
            VkViewport viewport{
                .width = static_cast<float>(pColourImageData->extent.width),
                .height = static_cast<float>(pColourImageData->extent.height),
                .minDepth = 0.f,
                .maxDepth = 1.f,
            };
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{
                .offset = VkOffset2D{},
                .extent = pColourImageData->extent,
            };
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            // vkDepthBias ??
        }

        { // Begin RenderPass
            std::array<VkClearValue, 2> clearValues{
                VkClearValue{.color = {1.f, 0.5f, 1.f, 0.f}},
                VkClearValue{.depthStencil =
                                 {
                                     .depth = 1.f,
                                     .stencil = 0,
                                 }},
            };

            VkRenderPassBeginInfo renderPassBI{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = renderPass,
                .framebuffer = framebuffer,
                .renderArea =
                    {
                        .offset = {0, 0},
                        .extent = pColourImageData->extent,
                    },
                .clearValueCount = clearValues.size(),
                .pClearValues = clearValues.data(),
            };

            vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
        }

        { // RENDER STUFF
            foeRenderSystem renderSystem = (foeRenderSystem)foeSimulationGetSystem(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM);

            auto renderDataSets = getRenderDataSets(renderSystem);
            VkDescriptorSet const *pPositionDescriptorSets =
                getPositionDescriptorSets(renderSystem, frameIndex);
            VkDescriptorSet const *pArmatureDescriptorSets =
                getArmatureDescriptorSets(renderSystem, frameIndex);

            auto *pDataSet = renderDataSets.data();
            auto const *pEnd = pDataSet + renderDataSets.size();

            for (; pDataSet != pEnd; ++pDataSet, ++pPositionDescriptorSets) {
                VkDescriptorSet armatureDescriptorSet = VK_NULL_HANDLE;
                if (pDataSet->armatureIndex != UINT32_MAX) {
                    armatureDescriptorSet = pArmatureDescriptorSets[pDataSet->armatureIndex];
                }

                renderCall(pDataSet, *pPositionDescriptorSets, armatureDescriptorSet, gfxSession,
                           commandBuffer, renderTargetSamples, renderPass, cameraDescriptor);
            }
        }

        { // End RenderPass
            vkCmdEndRenderPass(commandBuffer);
        }

        return vk_to_foeResult(vkResult);
    };

    // Resource Management
    struct RenderSceneJobResources {
        foeGfxVkGraphImageState initialColourImageState;
        foeGfxVkGraphImageState initialDepthStencilImageState;
        foeGfxVkGraphImageState colourImageState;
        foeGfxVkGraphImageState depthStencilImageState;
    };

    RenderSceneJobResources *pJobResources = new (std::nothrow) RenderSceneJobResources;
    if (pJobResources == nullptr)
        return to_foeResult(FOE_SKUNKWORKS_ERROR_OUT_OF_MEMORY);

    pJobResources->initialColourImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };
    pJobResources->initialDepthStencilImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    pJobResources->colourImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalColourLayout,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };
    pJobResources->depthStencilImageState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalDepthLayout,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1,
            },
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void { delete pJobResources; };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResourceState, 2> resourceStates{
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = colourRenderTargetUpstreamJobCount,
            .pUpstreamJobs = pColourRenderTargetUpstreamJobs,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = colourRenderTarget,
            .pIncomingState =
                (foeGfxVkRenderGraphStructure const *)&pJobResources->initialColourImageState,
            .pOutgoingState =
                (foeGfxVkRenderGraphStructure const *)&pJobResources->colourImageState,
        },
        foeGfxVkRenderGraphResourceState{
            .upstreamJobCount = depthRenderTargetUpstreamJobCount,
            .pUpstreamJobs = pDepthRenderTargetUpstreamJobs,
            .mode = RENDER_GRAPH_RESOURCE_MODE_READ_WRITE,
            .resource = depthRenderTarget,
            .pIncomingState =
                (foeGfxVkRenderGraphStructure const *)&pJobResources->initialDepthStencilImageState,
            .pOutgoingState =
                (foeGfxVkRenderGraphStructure const *)&pJobResources->depthStencilImageState,
        },
    };

    foeGfxVkRenderGraphJobInfo jobInfo{
        .resourceCount = resourceStates.size(),
        .pResources = resourceStates.data(),
        .freeDataFn = freeDataFn,
        .name = pJobName,
        .required = false,
        .fence = fence,
    };

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, &jobInfo, {}, std::move(jobFn), pRenderGraphJob);

    return result;
}