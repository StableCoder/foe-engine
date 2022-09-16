// Copyright (C) 2021-2022 George Cave.
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
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/session.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/simulation.hpp>

#include "../log.hpp"
#include "../result.h"
#include "../simulation/render_state_pool.hpp"
#include "../vk_result.h"

namespace {

auto renderCall(foeId entity,
                foeRenderState const *pRenderState,
                foeGfxSession gfxSession,
                foeSimulation *pSimulationSet,
                VkCommandBuffer commandBuffer,
                VkSampleCountFlags samples,
                VkRenderPass renderPass,
                VkDescriptorSet cameraDescriptor) -> bool {
    VkDescriptorSet const dummyDescriptorSet = foeGfxVkGetDummySet(gfxSession);

    foeResource vertexDescriptor{FOE_NULL_HANDLE};
    foeResource material{FOE_NULL_HANDLE};
    foeResource mesh{FOE_NULL_HANDLE};

    bool boned{false};
    if (pRenderState->bonedVertexDescriptor != FOE_INVALID_ID &&
        pRenderState->boneDescriptorSet != VK_NULL_HANDLE) {
        boned = true;

        do {
            vertexDescriptor = foeResourcePoolFind(pSimulationSet->resourcePool,
                                                   pRenderState->bonedVertexDescriptor);

            if (vertexDescriptor == FOE_NULL_HANDLE) {
                vertexDescriptor = foeResourcePoolAdd(
                    pSimulationSet->resourcePool, pRenderState->bonedVertexDescriptor,
                    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
                    sizeof(foeVertexDescriptor));
            }
        } while (vertexDescriptor == FOE_NULL_HANDLE);
    } else {
        do {
            vertexDescriptor =
                foeResourcePoolFind(pSimulationSet->resourcePool, pRenderState->vertexDescriptor);

            if (vertexDescriptor == FOE_NULL_HANDLE) {
                vertexDescriptor =
                    foeResourcePoolAdd(pSimulationSet->resourcePool, pRenderState->vertexDescriptor,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
                                       sizeof(foeVertexDescriptor));
            }
        } while (vertexDescriptor == FOE_NULL_HANDLE);
    }

    do {
        material = foeResourcePoolFind(pSimulationSet->resourcePool, pRenderState->material);

        if (material == FOE_NULL_HANDLE) {
            material = foeResourcePoolAdd(pSimulationSet->resourcePool, pRenderState->material,
                                          FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL,
                                          sizeof(foeMaterial));
        }
    } while (material == FOE_NULL_HANDLE);

    do {
        mesh = foeResourcePoolFind(pSimulationSet->resourcePool, pRenderState->mesh);

        if (mesh == FOE_NULL_HANDLE) {
            mesh = foeResourcePoolAdd(pSimulationSet->resourcePool, pRenderState->mesh,
                                      FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH, sizeof(foeMesh));
        }
    } while (mesh == FOE_NULL_HANDLE);

    if (vertexDescriptor == FOE_NULL_HANDLE || material == FOE_NULL_HANDLE ||
        mesh == FOE_NULL_HANDLE) {
        return false;
    }

    bool skip = false;
    if (auto loadState = foeResourceGetState(vertexDescriptor);
        loadState != FOE_RESOURCE_LOAD_STATE_LOADED) {
        if (loadState == FOE_RESOURCE_LOAD_STATE_UNLOADED &&
            !foeResourceGetIsLoading(vertexDescriptor)) {
            FOE_LOG(foeBringup, Verbose,
                    "While attempting to render {}, VertexDeescriptor resource {} was unloaded and "
                    "wasn't being loaded, requesting load",
                    foeIdToString(entity), foeIdToString(foeResourceGetID(vertexDescriptor)));
            foeResourceLoadData(vertexDescriptor);
        }

        skip = true;
    }

    if (auto loadState = foeResourceGetState(material);
        loadState != FOE_RESOURCE_LOAD_STATE_LOADED) {
        if (loadState == FOE_RESOURCE_LOAD_STATE_UNLOADED && !foeResourceGetIsLoading(material)) {
            FOE_LOG(foeBringup, Verbose,
                    "While attempting to render {}, Material resource {} was unloaded and wasn't "
                    "being loaded, requesting load",
                    foeIdToString(entity), foeIdToString(foeResourceGetID(material)));
            foeResourceLoadData(material);
        }

        skip = true;
    }

    if (auto loadState = foeResourceGetState(mesh); loadState != FOE_RESOURCE_LOAD_STATE_LOADED) {
        if (loadState == FOE_RESOURCE_LOAD_STATE_UNLOADED && !foeResourceGetIsLoading(mesh)) {
            FOE_LOG(foeBringup, Verbose,
                    "While attempting to render {}, Mesh resource {} was unloaded and wasn't being "
                    "loaded, requesting load",
                    foeIdToString(entity), foeIdToString(foeResourceGetID(mesh)));
            foeResourceLoadData(mesh);
        }

        skip = true;
    }
    if (skip)
        return false;

    // Get Resource Data
    auto const *pVertexDescriptor =
        (foeVertexDescriptor const *)foeResourceGetData(vertexDescriptor);
    auto const *pMaterial = (foeMaterial const *)foeResourceGetData(material);
    auto const *pMesh = (foeMesh const *)foeResourceGetData(mesh);

    // Retrieve the pipeline
    auto *pGfxVertexDescriptor = &pVertexDescriptor->vertexDescriptor;
    VkPipelineLayout layout;
    uint32_t descriptorSetLayoutCount;
    VkPipeline pipeline;

    auto pipelinePool = foeGfxVkGetPipelinePool(gfxSession);
    auto result = foeGfxVkGetPipeline(pipelinePool,
                                      const_cast<foeGfxVertexDescriptor *>(pGfxVertexDescriptor),
                                      pMaterial->pGfxFragDescriptor, renderPass, 0, samples,
                                      &layout, &descriptorSetLayoutCount, &pipeline);
    if (result.value != FOE_SUCCESS)
        std::abort();

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            &cameraDescriptor, 0, nullptr);

    foeGfxVkBindMesh(pMesh->gfxData, commandBuffer, boned);

    auto vertSetLayouts = pGfxVertexDescriptor->getBuiltinSetLayouts();
    if (vertSetLayouts & FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulationSet, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto posOffset = pPosition3dPool->find(entity);

        // If can't find a position, return failure
        if (posOffset == pPosition3dPool->size())
            return false;

        auto *pPosition = (pPosition3dPool->begin<1>() + posOffset)->get();
        // Bind the object's position *if* the descriptor supports it
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &pPosition->descriptorSet, 0, nullptr);
    } else {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &dummyDescriptorSet, 0, nullptr);
    }
    if (boned) {
        // If we have bone information, bind that too
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
                                &pRenderState->boneDescriptorSet, 0, nullptr);
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

void destroy_VkCommandPool(VkCommandPool commandPool, foeGfxSession session) {
    vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
}

} // namespace

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
                            RenderSceneOutputResources &outputResources) {
    // Make sure the resources passed in are images, and are mutable
    auto const *pColourImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        colourRenderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto const *pDepthImageData = (foeGfxVkGraphImageResource const *)foeGfxVkGraphFindStructure(
        depthRenderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pColourImageData == nullptr) {
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE);
    }
    if (!pColourImageData->isMutable) {
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE);
    }
    if (pDepthImageData == nullptr) {
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE);
    }
    if (!pDepthImageData->isMutable) {
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE);
    }

    // Check that the images have previous state
    auto const *pColourImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        colourRenderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto const *pDepthImageState = (foeGfxVkGraphImageState const *)foeGfxVkGraphFindStructure(
        depthRenderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pColourImageState == nullptr)
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE);
    if (pDepthImageState == nullptr)
        return to_foeResult(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE);

    // Proceed with the job
    auto jobFn = [=](foeGfxSession gfxSession, foeGfxDelayedCaller gfxDelayedDestructor,
                     std::vector<VkSemaphore> const &waitSemaphores,
                     std::vector<VkSemaphore> const &signalSemaphores,
                     std::function<void(std::function<void()>)> addCpuFnFn) -> foeResultSet {
        VkResult vkResult;

        VkRenderPass renderPass =
            foeGfxVkGetRenderPassPool(gfxSession)
                ->renderPass(
                    {VkAttachmentDescription{
                         .format = pColourImageData->format,
                         .samples = static_cast<VkSampleCountFlagBits>(renderTargetSamples),
                         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                         .initialLayout = pColourImageState->layout,
                         .finalLayout = finalColourLayout,
                     },
                     VkAttachmentDescription{
                         .format = pDepthImageData->format,
                         .samples = static_cast<VkSampleCountFlagBits>(renderTargetSamples),
                         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                         .initialLayout = pDepthImageState->layout,
                         .finalLayout = finalDepthLayout,
                     }});

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

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        { // Create CommandPool
            VkCommandPoolCreateInfo poolCI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = 0,
            };
            vkResult =
                vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr, &commandPool);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_VkCommandPool,
                                        (void *)commandPool);
        }

        { // Create CommandBuffer
            VkCommandBufferAllocateInfo commandBufferAI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkResult = vkAllocateCommandBuffers(foeGfxVkGetDevice(gfxSession), &commandBufferAI,
                                                &commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        { // Begin CommandBuffer
            VkCommandBufferBeginInfo commandBufferBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
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
            auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

            auto idIt = pRenderStatePool->cbegin();
            auto const endIdIt = pRenderStatePool->cend();
            auto dataIt = pRenderStatePool->cbegin<1>();
            for (; idIt != endIdIt; ++idIt, ++dataIt) {
                renderCall(*idIt, dataIt, gfxSession, pSimulation, commandBuffer,
                           renderTargetSamples, renderPass, cameraDescriptor);
            }
        }

        { // End RenderPass
            vkCmdEndRenderPass(commandBuffer);
        }

        { // End CommandBuffer
            vkResult = vkEndCommandBuffer(commandBuffer);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);
        }

        // Job Submission
        std::vector<VkPipelineStageFlags> waitMasks(waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitMasks.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
        vkResult = vkQueueSubmit(queue, 1, &submitInfo, fence);
        foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

        return vk_to_foeResult(vkResult);
    };

    // Resource Management
    auto *pNewColourState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalColourLayout,
    };
    auto *pNewDepthState = new (std::nothrow) foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalDepthLayout,
    };

    foeGfxVkRenderGraphFn freeDataFn = [=]() -> void {
        if (pNewColourState)
            delete pNewColourState;
        if (pNewDepthState)
            delete pNewDepthState;
    };

    if (pNewColourState == nullptr || pNewDepthState == nullptr) {
        freeDataFn();
        return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
    }

    // Add job to graph
    std::array<foeGfxVkRenderGraphResource const, 2> resourcesIn{colourRenderTarget,
                                                                 depthRenderTarget};
    std::array<bool const, 2> resourcesInReadOnly{false, false};
    foeGfxVkRenderGraphJob renderGraphJob;

    foeResultSet result =
        foeGfxVkRenderGraphAddJob(renderGraph, 2, resourcesIn.data(), resourcesInReadOnly.data(),
                                  freeDataFn, name, false, std::move(jobFn), &renderGraphJob);
    if (result.value != FOE_SUCCESS)
        return result;

    // Outgoing state
    outputResources = RenderSceneOutputResources{
        .colourRenderTarget =
            {
                .provider = renderGraphJob,
                .pResourceData = colourRenderTarget.pResourceData,
                .pResourceState = (foeGfxVkRenderGraphStructure const *)pNewColourState,
            },
        .depthRenderTarget =
            {
                .provider = renderGraphJob,
                .pResourceData = depthRenderTarget.pResourceData,
                .pResourceState = (foeGfxVkRenderGraphStructure const *)pNewDepthState,
            },
    };

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}