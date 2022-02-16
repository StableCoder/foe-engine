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

#include "render_scene.hpp"

#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/graphics/vk/mesh.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/graphics/vk/render_graph/resource/image.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/simulation.hpp>
#include <vk_error_code.hpp>

#include "../camera_pool.hpp"
#include "../error_code.hpp"
#include "../render_state_pool.hpp"

namespace {

template <typename ResourcePool>
auto getResourcePool(foeResourcePoolBase **pResourcePools, size_t poolCount) -> ResourcePool * {
    ResourcePool *pPool{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pPool = dynamic_cast<ResourcePool *>(pResourcePools[i]);
        if (pPool != nullptr)
            break;
    }

    return pPool;
}

template <typename ComponentPool>
auto getComponentPool(foeComponentPoolBase **pComponentPools, size_t poolCount) -> ComponentPool * {
    ComponentPool *pPool{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pPool = dynamic_cast<ComponentPool *>(pComponentPools[i]);
        if (pPool != nullptr)
            break;
    }

    return pPool;
}

auto renderCall(foeId entity,
                foeRenderState const *pRenderState,
                foeGfxSession gfxSession,
                foeSimulationState *pSimulationSet,
                VkCommandBuffer commandBuffer,
                VkSampleCountFlags samples,
                VkRenderPass renderPass,
                VkDescriptorSet cameraDescriptor) -> bool {
    VkDescriptorSet const dummyDescriptorSet = foeGfxVkGetDummySet(gfxSession);

    foeVertexDescriptor *pVertexDescriptor{nullptr};
    bool boned{false};
    if (pRenderState->bonedVertexDescriptor != FOE_INVALID_ID &&
        pRenderState->boneDescriptorSet != VK_NULL_HANDLE) {
        boned = true;
        pVertexDescriptor =
            getResourcePool<foeVertexDescriptorPool>(pSimulationSet->resourcePools.data(),
                                                     pSimulationSet->resourcePools.size())
                ->find(pRenderState->bonedVertexDescriptor);
    }

    if (pVertexDescriptor == nullptr) {
        pVertexDescriptor =
            getResourcePool<foeVertexDescriptorPool>(pSimulationSet->resourcePools.data(),
                                                     pSimulationSet->resourcePools.size())
                ->find(pRenderState->vertexDescriptor);
    }

    auto *pMaterial = getResourcePool<foeMaterialPool>(pSimulationSet->resourcePools.data(),
                                                       pSimulationSet->resourcePools.size())
                          ->find(pRenderState->material);
    auto *pMesh = getResourcePool<foeMeshPool>(pSimulationSet->resourcePools.data(),
                                               pSimulationSet->resourcePools.size())
                      ->find(pRenderState->mesh);

    if (pVertexDescriptor == nullptr || pMaterial == nullptr || pMesh == nullptr) {
        return false;
    }
    if (pVertexDescriptor->getState() != foeResourceState::Loaded ||
        pMaterial->getState() != foeResourceState::Loaded ||
        pMesh->getState() != foeResourceState::Loaded) {
        return false;
    }

    // Retrieve the pipeline
    auto *pGfxVertexDescriptor = &pVertexDescriptor->data.vertexDescriptor;
    VkPipelineLayout layout;
    uint32_t descriptorSetLayoutCount;
    VkPipeline pipeline;

    foeGfxVkGetPipelinePool(gfxSession)
        ->getPipeline(const_cast<foeGfxVertexDescriptor *>(pGfxVertexDescriptor),
                      pMaterial->data.pGfxFragDescriptor, renderPass, 0, samples, &layout,
                      &descriptorSetLayoutCount, &pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            &cameraDescriptor, 0, nullptr);

    foeGfxVkBindMesh(pMesh->data.gfxData, commandBuffer, boned);

    auto vertSetLayouts = pGfxVertexDescriptor->getBuiltinSetLayouts();
    if (vertSetLayouts & FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX) {
        auto *pPosition3dPool = getComponentPool<foePosition3dPool>(
            pSimulationSet->componentPools.data(), pSimulationSet->componentPools.size());

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
    if (auto set = pMaterial->data.materialDescriptorSet; set != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                foeDescriptorSetLayoutIndex::FragmentShader, 1, &set, 0, nullptr);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDrawIndexed(commandBuffer, foeGfxGetMeshIndices(pMesh->data.gfxData), 1, 0, 0, 0);

    return true;
}
} // namespace

auto renderSceneJob(foeGfxVkRenderGraph renderGraph,
                    std::string_view name,
                    VkFence fence,
                    foeGfxVkRenderGraphResource colourRenderTarget,
                    VkImageLayout finalColourLayout,
                    foeGfxVkRenderGraphResource depthRenderTarget,
                    VkImageLayout finalDepthLayout,
                    VkSampleCountFlags renderTargetSamples,
                    foeSimulationState *pSimulationState,
                    VkDescriptorSet cameraDescriptor,
                    RenderSceneOutputResources &outputResources) -> std::error_code {
    std::error_code errC;

    // Make sure the resources passed in are images, and are mutable
    auto *pColourImageData = (foeGfxVkGraphImageResource *)foeGfxVkGraphFindStructure(
        colourRenderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);
    auto *pDepthImageData = (foeGfxVkGraphImageResource *)foeGfxVkGraphFindStructure(
        depthRenderTarget.pResourceData, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE);

    if (pColourImageData == nullptr) {
        return FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE;
    }
    if (!pColourImageData->isMutable) {
        return FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE;
    }
    if (pDepthImageData == nullptr) {
        return FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE;
    }
    if (!pDepthImageData->isMutable) {
        return FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE;
    }

    // Check that the images have previous state
    auto *pColourImageState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        colourRenderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);
    auto *pDepthImageState = (foeGfxVkGraphImageState *)foeGfxVkGraphFindStructure(
        depthRenderTarget.pResourceState, RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE);

    if (pColourImageState == nullptr)
        return FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE;
    if (pDepthImageState == nullptr)
        return FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE;

    // Proceed with the job
    auto *pJob = new RenderGraphJob;
    *pJob = RenderGraphJob{
        .name = std::string{name},
        .required = false,
        .executeFn = [=](foeGfxSession gfxSession, foeGfxDelayedDestructor gfxDelayedDestructor,
                         std::vector<VkSemaphore> const &waitSemaphores,
                         std::vector<VkSemaphore> const &signalSemaphores,
                         std::function<void(std::function<void()>)> addCpuFnFn) -> std::error_code {
            std::error_code errC;

            auto *pColourRenderTarget =
                reinterpret_cast<foeGfxVkGraphImageResource *>(colourRenderTarget.pResourceData);
            auto *pDepthRenderTarget =
                reinterpret_cast<foeGfxVkGraphImageResource *>(depthRenderTarget.pResourceData);

            VkRenderPass renderPass =
                foeGfxVkGetRenderPassPool(gfxSession)
                    ->renderPass(
                        {VkAttachmentDescription{
                             .format = pColourRenderTarget->format,
                             .samples = static_cast<VkSampleCountFlagBits>(renderTargetSamples),
                             .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                             .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                             .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                             .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                             .initialLayout = pColourImageState->layout,
                             .finalLayout = finalColourLayout,
                         },
                         VkAttachmentDescription{
                             .format = pDepthRenderTarget->format,
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
                std::array<VkImageView, 2> renderTargetViews = {pColourRenderTarget->view,
                                                                pDepthRenderTarget->view};
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = renderTargetViews.size(),
                    .pAttachments = renderTargetViews.data(),
                    .width = pColourRenderTarget->extent.width,
                    .height = pColourRenderTarget->extent.height,
                    .layers = 1,
                };
                errC = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr,
                                           &framebuffer);
                if (errC)
                    return errC;

                foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                    vkDestroyFramebuffer(foeGfxVkGetDevice(session), framebuffer, nullptr);
                });
            }

            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;

            { // Create CommandPool
                VkCommandPoolCreateInfo poolCI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .queueFamilyIndex = 0,
                };
                errC = vkCreateCommandPool(foeGfxVkGetDevice(gfxSession), &poolCI, nullptr,
                                           &commandPool);
                if (errC)
                    return errC;

                foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                    vkDestroyCommandPool(foeGfxVkGetDevice(session), commandPool, nullptr);
                });
            }

            { // Create CommandBuffer
                VkCommandBufferAllocateInfo commandBufferAI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = commandPool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1,
                };

                errC = vkAllocateCommandBuffers(foeGfxVkGetDevice(gfxSession), &commandBufferAI,
                                                &commandBuffer);
                if (errC)
                    return errC;
            }

            { // Begin CommandBuffer
                VkCommandBufferBeginInfo commandBufferBI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };
                errC = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                if (errC)
                    return errC;
            }

            { // Setup common render viewport data
                VkViewport viewport{
                    .width = static_cast<float>(pColourRenderTarget->extent.width),
                    .height = static_cast<float>(pColourRenderTarget->extent.height),
                    .minDepth = 0.f,
                    .maxDepth = 1.f,
                };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{
                    .offset = VkOffset2D{},
                    .extent = pColourRenderTarget->extent,
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
                            .extent = pColourRenderTarget->extent,
                        },
                    .clearValueCount = clearValues.size(),
                    .pClearValues = clearValues.data(),
                };

                vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
            }

            { // RENDER STUFF
                auto *pRenderStatePool =
                    getComponentPool<foeRenderStatePool>(pSimulationState->componentPools.data(),
                                                         pSimulationState->componentPools.size());

                auto idIt = pRenderStatePool->cbegin();
                auto const endIdIt = pRenderStatePool->cend();
                auto dataIt = pRenderStatePool->cbegin<1>();
                for (; idIt != endIdIt; ++idIt, ++dataIt) {
                    renderCall(*idIt, dataIt, gfxSession, pSimulationState, commandBuffer,
                               renderTargetSamples, renderPass, cameraDescriptor);
                }
            }

            { // End RenderPass
                vkCmdEndRenderPass(commandBuffer);
            }

            { // End CommandBuffer
                errC = vkEndCommandBuffer(commandBuffer);
                if (errC)
                    return errC;
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
            errC = vkQueueSubmit(queue, 1, &submitInfo, fence);
            foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);

            return errC;
        },
    };

    // Resource Management
    auto *pNewColourState = new foeGfxVkGraphImageState;
    *pNewColourState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalColourLayout,
    };
    auto *pNewDepthState = new foeGfxVkGraphImageState;
    *pNewDepthState = foeGfxVkGraphImageState{
        .sType = RENDER_GRAPH_RESOURCE_STRUCTURE_TYPE_IMAGE_STATE,
        .layout = finalDepthLayout,
    };

    DeleteResourceDataCall deleteCalls[2] = {
        {
            .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
                delete reinterpret_cast<foeGfxVkGraphImageState *>(pResource);
            },
            .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pNewColourState),
        },
        {
            .deleteFn = [](foeGfxVkGraphStructure *pResource) -> void {
                delete reinterpret_cast<foeGfxVkGraphImageState *>(pResource);
            },
            .pResource = reinterpret_cast<foeGfxVkGraphStructure *>(pNewDepthState),
        },
    };

    // Add job to graph
    std::array<foeGfxVkRenderGraphResource const, 2> resourcesIn{colourRenderTarget,
                                                                 depthRenderTarget};
    std::array<bool const, 2> resourcesInReadOnly{false, false};

    errC = foeGfxVkRenderGraphAddJob(renderGraph, pJob, 2, resourcesIn.data(),
                                     resourcesInReadOnly.data(), 2, deleteCalls);
    if (errC)
        return errC;

    // Outgoing state
    outputResources = RenderSceneOutputResources{
        .colourRenderTarget =
            {
                .pProvider = pJob,
                .pResourceData = colourRenderTarget.pResourceData,
                .pResourceState = (foeGfxVkGraphStructure *)pNewColourState,
            },
        .depthRenderTarget =
            {
                .pProvider = pJob,
                .pResourceData = depthRenderTarget.pResourceData,
                .pResourceState = (foeGfxVkGraphStructure *)pNewDepthState,
            },
    };

    return FOE_BRINGUP_SUCCESS;
}