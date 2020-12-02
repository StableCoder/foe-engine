/*
    Copyright (C) 2020 George Cave.

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

#include <GLFW/glfw3.h>
#include <foe/developer_console.hpp>
#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/descriptor_set_layout_pool.hpp>
#include <foe/graphics/environment.hpp>
#include <foe/graphics/fragment_descriptor.hpp>
#include <foe/graphics/fragment_descriptor_pool.hpp>
#include <foe/graphics/pipeline_pool.hpp>
#include <foe/graphics/render_pass_pool.hpp>
#include <foe/graphics/resource_uploader.hpp>
#include <foe/graphics/shader_pool.hpp>
#include <foe/graphics/swapchain.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/vertex_descriptor.hpp>
#include <foe/log.hpp>
#include <foe/wsi.hpp>
#include <foe/wsi_vulkan.hpp>
#include <vk_error_code.hpp>

#include <array>
#include <chrono>
#include <iostream>

#include "camera.hpp"
#include "camera_descriptor_pool.hpp"
#include "frame_timer.hpp"
#include "stdout_sink.hpp"

#ifdef EDITOR_MODE
#include <foe/imgui/renderer.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include "imgui/frame_time_info.hpp"
#include "imgui/termination.hpp"
#endif

#define VK_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = res;                                                                \
        FOE_LOG(General, Fatal, "End called from {}:{} with Vulkan error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{}", __FILE__, __LINE__);                      \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

using namespace std::chrono_literals;

struct VkWindowData {
    VkSurfaceKHR surface{VK_NULL_HANDLE};
};

void destroyVkWindowData(VkWindowData *pData, VkInstance instance, VkDevice device) {
    if (pData->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, pData->surface, nullptr);
}

struct PerFrameData {
    VkSemaphore presentImageAcquired;
    VkSemaphore renderComplete;

    VkFence frameComplete;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkResult create(VkDevice device) noexcept {
        // Semaphores
        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        auto res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentImageAcquired);
        if (res != VK_SUCCESS)
            return res;

        res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &renderComplete);
        if (res != VK_SUCCESS)
            return res;

        // Fences
        VkFenceCreateInfo fenceCI{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        res = vkCreateFence(device, &fenceCI, nullptr, &frameComplete);
        if (res != VK_SUCCESS)
            return res;

        // Command Pools
        VkCommandPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        };
        res = vkCreateCommandPool(device, &poolCI, nullptr, &commandPool);
        if (res != VK_SUCCESS)
            return res;

        // Command Buffers
        VkCommandBufferAllocateInfo commandBufferAI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        res = vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);
        if (res != VK_SUCCESS)
            return res;

        return res;
    }

    void destroy(VkDevice device) {
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyFence(device, frameComplete, nullptr);

        vkDestroySemaphore(device, renderComplete, nullptr);
        vkDestroySemaphore(device, presentImageAcquired, nullptr);
    }
};

int main(int, char **) {
    StdOutSink stdoutSink;
    foeLogger::instance()->registerSink(&stdoutSink);
    foeLogger::instance()->registerSink(foeDeveloperConsole::instance());

    FrameTimer frameTime;
    foeGfxEnvironment *pGfxEnvironment;
    foeResourceUploader resUploader;

    VkWindowData vkWindow;
    foeSwapchain swapchain;

    uint32_t frameIndex = 0;
    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

    foeVertexDescriptor vertexDescriptor;
    foeFragmentDescriptor *fragmentDescriptor{nullptr};

    foeRenderPassPool renderPassPool;

    foeDescriptorSetLayoutPool descriptorSetLayoutPool;
    foeBuiltinDescriptorSets builtinDescriptorSets;
    foeShaderPool shaderPool;
    foeFragmentDescriptorPool fragmentDescriptorPool;
    foePipelinePool pipelinePool;

    Camera camera;
    CameraDescriptorPool cameraDescriptorPool;

    cameraDescriptorPool.linkCamera(&camera);

#ifdef EDITOR_MODE
    foeImGuiRenderer imguiRenderer;
    foeImGuiState imguiState;

    foeImGuiTermination fileTermination;
    foeImGuiFrameTimeInfo viewFrameTimeInfo{&frameTime};

    imguiState.addUI(&fileTermination);
    imguiState.addUI(&viewFrameTimeInfo);
#endif

    std::vector<VkFramebuffer> swapImageFramebuffers;
    bool swapchainRebuilt = false;

    VkResult res = foeGfxCreateEnvironment(false, "FoE Engine", 0, &pGfxEnvironment);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = foeGfxCreateResourceUploader(pGfxEnvironment, &resUploader);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    if (!foeCreateWindow(1280, 720, "FoE Engine"))
        END_PROGRAM

#ifdef EDITOR_MODE
    imguiRenderer.resize(1280, 720);
    float xScale, yScale;
    foeWindowGetContentScale(&xScale, &yScale);
    imguiRenderer.rescale(xScale, yScale);
#endif

    res = foeWindowGetVkSurface(pGfxEnvironment->instance, &vkWindow.surface);
    if (res != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    for (auto &it : frameData) {
        res = it.create(pGfxEnvironment->device);
        if (res != VK_SUCCESS) {
            VK_END_PROGRAM
        }
    }

    res = renderPassPool.initialize(pGfxEnvironment->device);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = descriptorSetLayoutPool.initialize(pGfxEnvironment->device);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = builtinDescriptorSets.initialize(pGfxEnvironment->device, &descriptorSetLayoutPool);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = shaderPool.initialize(pGfxEnvironment->device);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = pipelinePool.initialize(pGfxEnvironment->device, &builtinDescriptorSets);
    if (res != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    res = cameraDescriptorPool.initialize(
        pGfxEnvironment,
        builtinDescriptorSets.getBuiltinLayout(
            foeBuiltinDescriptorSetLayoutFlagBits::
                FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX),
        builtinDescriptorSets.getBuiltinSetLayoutIndex(
            foeBuiltinDescriptorSetLayoutFlagBits::
                FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX));
    if (res != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    {
        foeShader *pShader;

        pShader = shaderPool.create("data/shaders/simple/tri.vert.spv");
        vertexDescriptor.mVertex = pShader;
        pShader->incrementUseCount();

        pShader = shaderPool.create("data/shaders/simple/uv_to_colour.frag.spv");
        pShader->incrementUseCount();

        // Vertex
        vertexDescriptor.mVertexInputSCI = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };
        vertexDescriptor.mInputAssemblySCI = VkPipelineInputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        };

        // Fragment
        auto rasterization = VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .lineWidth = 1.0f,
        };
        std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments{
            VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            }};
        auto colourBlend = VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(colourBlendAttachments.size()),
            .pAttachments = colourBlendAttachments.data(),
        };

        fragmentDescriptor =
            fragmentDescriptorPool.get(&rasterization, nullptr, &colourBlend, pShader);
    }

    // MAIN LOOP BEGIN
    FOE_LOG(General, Info, "Entering main loop")
    while (!foeWindowGetShouldClose()
#ifdef EDITOR_MODE
           && !fileTermination.terminationRequested()
#endif
    ) {
        swapchainRebuilt = false;
        foeWindowEventProcessing();

        auto *pMouse = foeGetMouse();
        auto *pKeyboard = foeGetKeyboard();

#ifdef EDITOR_MODE
        // User input processing
        imguiRenderer.keyboardInput(pKeyboard);
        imguiRenderer.mouseInput(pMouse);
#endif

        if (foeWindowResized()) {
            // Swapchins will need rebuilding
            swapchain.requestRebuild();

#ifdef EDITOR_MODE
            int width, height;
            foeWindowGetSize(&width, &height);
            imguiRenderer.resize(width, height);
#endif
        }

        // Vulkan Render Section
        uint32_t nextFrameIndex = (frameIndex + 1) % frameData.size();
        if (VK_SUCCESS == vkWaitForFences(pGfxEnvironment->device, 1,
                                          &frameData[nextFrameIndex].frameComplete, VK_TRUE, 0)) {
            frameTime.newFrame();

            // Rebuild swapchains
            if (!swapchain || swapchain.needRebuild()) {
                // If no swapchain, then that means we need to get the surface format and
                // presentation mode first
                if (!swapchain) {
                    { // Present Queues
                        for (int i = 0; i < pGfxEnvironment->numQueueFamilies; ++i) {
                            VkBool32 support = VK_FALSE;
                            vkGetPhysicalDeviceSurfaceSupportKHR(pGfxEnvironment->physicalDevice, i,
                                                                 vkWindow.surface, &support);
                            FOE_LOG(General, Info,
                                    "Support on queue family {} for surface presentation: {}", i,
                                    (bool)support);
                        }
                    }

                    { // Surface Formats
                        uint32_t formatCount;
                        res = vkGetPhysicalDeviceSurfaceFormatsKHR(pGfxEnvironment->physicalDevice,
                                                                   vkWindow.surface, &formatCount,
                                                                   nullptr);
                        if (res != VK_SUCCESS)
                            VK_END_PROGRAM

                        std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats(
                            new VkSurfaceFormatKHR[formatCount]);

                        res = vkGetPhysicalDeviceSurfaceFormatsKHR(pGfxEnvironment->physicalDevice,
                                                                   vkWindow.surface, &formatCount,
                                                                   surfaceFormats.get());
                        if (res != VK_SUCCESS)
                            VK_END_PROGRAM

                        swapchain.surfaceFormat(surfaceFormats.get()[0]);
                    }

                    { // Present Modes
                        uint32_t modeCount;
                        vkGetPhysicalDeviceSurfacePresentModesKHR(
                            pGfxEnvironment->physicalDevice, vkWindow.surface, &modeCount, nullptr);

                        std::unique_ptr<VkPresentModeKHR[]> presentModes(
                            new VkPresentModeKHR[modeCount]);

                        vkGetPhysicalDeviceSurfacePresentModesKHR(pGfxEnvironment->physicalDevice,
                                                                  vkWindow.surface, &modeCount,
                                                                  presentModes.get());

                        swapchain.presentMode(presentModes.get()[0]);
                    }
                }

                foeSwapchain newSwapchain;

                int width, height;
                foeWindowGetSize(&width, &height);
                res = newSwapchain.create(pGfxEnvironment->physicalDevice, pGfxEnvironment->device,
                                          vkWindow.surface, swapchain.surfaceFormat(),
                                          swapchain.presentMode(), swapchain, 3, width, height);
                if (res != VK_SUCCESS)
                    VK_END_PROGRAM

                // If the old swapchain exists, we need to destroy it
                if (swapchain) {
                    swapchain.destroy(pGfxEnvironment->device);
                }

                swapchain = newSwapchain;
                swapchainRebuilt = true;
            }

            // Acquire Target Presentation Images
            VkResult res = swapchain.acquireNextImage(
                pGfxEnvironment->device, frameData[nextFrameIndex].presentImageAcquired);
            if (res == VK_TIMEOUT || res == VK_NOT_READY) {
                // Waiting for an image to become ready
                goto SKIP_FRAME_RENDER;
            } else if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
                // Surface changed, best to rebuild swapchains
                goto SKIP_FRAME_RENDER;
            } else if (res != VK_SUCCESS) {
                // Catastrophic error
                VK_END_PROGRAM
            }
            vkResetFences(pGfxEnvironment->device, 1, &frameData[nextFrameIndex].frameComplete);
            frameIndex = nextFrameIndex;

            // Set camera descriptors
            cameraDescriptorPool.generateCameraDescriptors(frameIndex);

            // Rendering
            vkResetCommandPool(pGfxEnvironment->device, frameData[frameIndex].commandPool, 0);

            // Render passes
            VkRenderPass swapImageRenderPass = renderPassPool.renderPass({VkAttachmentDescription{
                .format = swapchain.surfaceFormat().format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            }});

#ifdef EDITOR_MODE
            if (!imguiRenderer.initialized()) {
                if (imguiRenderer.initialize(pGfxEnvironment, VK_SAMPLE_COUNT_1_BIT,
                                             swapImageRenderPass, 0) != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }
#endif

            if (swapchainRebuilt) {
                for (auto &it : swapImageFramebuffers)
                    vkDestroyFramebuffer(pGfxEnvironment->device, it, nullptr);
                swapImageFramebuffers.clear();

                int width, height;
                foeWindowGetSize(&width, &height);
                VkImageView view;
                VkExtent2D swapchainExtent = swapchain.extent();
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = swapImageRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = &view,
                    .width = (uint32_t)swapchainExtent.width,
                    .height = (uint32_t)swapchainExtent.height,
                    .layers = 1,
                };

                for (uint32_t i = 0; i < swapchain.chainSize(); ++i) {
                    view = swapchain.imageView(i);

                    VkFramebuffer framebuffer;
                    res = vkCreateFramebuffer(pGfxEnvironment->device, &framebufferCI, nullptr,
                                              &framebuffer);
                    if (res != VK_SUCCESS)
                        VK_END_PROGRAM
                    swapImageFramebuffers.emplace_back(framebuffer);
                }
            }

            {
                VkCommandBuffer &commandBuffer = frameData[frameIndex].commandBuffer;

                VkCommandBufferBeginInfo commandBufferBI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };

                res = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                if (res != VK_SUCCESS)
                    goto SHUTDOWN_PROGRAM;

                { // Render Pass Setup
                    VkExtent2D swapchainExtent = swapchain.extent();
                    VkClearValue clear{
                        .color = {0.f, 0.f, 1.f, 0.f},
                    };

                    VkRenderPassBeginInfo renderPassBI{
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = swapImageRenderPass,
                        .framebuffer = swapImageFramebuffers[swapchain.acquiredIndex()],
                        .renderArea =
                            {
                                .offset = {0, 0},
                                .extent = swapchainExtent,
                            },
                        .clearValueCount = 1,
                        .pClearValues = &clear,
                    };

                    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

                    { // Set Drawing Parameters
                        VkViewport viewport{
                            .width = static_cast<float>(swapchainExtent.width),
                            .height = static_cast<float>(swapchainExtent.height),
                            .minDepth = 0.f,
                            .maxDepth = 1.f,
                        };
                        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                        VkRect2D scissor{
                            .offset = VkOffset2D{},
                            .extent = swapchainExtent,
                        };
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                        // If we had depthbias enabled
                        // vkCmdSetDepthBias

                        // Set the Pipeline
                        VkPipelineLayout layout;
                        uint32_t descriptorSetLayoutCount;
                        VkPipeline pipeline;

                        pipelinePool.getPipeline(&vertexDescriptor, fragmentDescriptor,
                                                 swapImageRenderPass, 0, &layout,
                                                 &descriptorSetLayoutCount, &pipeline);

                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                        vkCmdDraw(commandBuffer, 4, 1, 0, 0);
                    }

#ifdef EDITOR_MODE
                    { // ImGui
                        imguiRenderer.newFrame();
                        imguiState.runUI();
                        imguiRenderer.endFrame();

                        res = imguiRenderer.update(pGfxEnvironment->allocator, frameIndex);
                        if (res != VK_SUCCESS) {
                            VK_END_PROGRAM
                        }

                        imguiRenderer.draw(commandBuffer, frameIndex);
                    }
#endif

                    vkCmdEndRenderPass(commandBuffer);
                }

                res = vkEndCommandBuffer(commandBuffer);
                if (res != VK_SUCCESS) {
                    FOE_LOG(General, Fatal, "Error with drawing: {}",
                            std::error_code{res}.message());
                }

                // Submission
                VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                VkSubmitInfo submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &frameData[frameIndex].presentImageAcquired,
                    .pWaitDstStageMask = &waitMask,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &frameData[frameIndex].renderComplete,
                };

                res = vkQueueSubmit(pGfxEnvironment->pQueueFamilies[0].queue[0], 1, &submitInfo,
                                    frameData[frameIndex].frameComplete);
                if (res != VK_SUCCESS)
                    goto SHUTDOWN_PROGRAM;
            }

            // Presentation
            {
                std::vector<VkSwapchainKHR> swapchains;
                std::vector<uint32_t> swapchainIndices;
                std::vector<VkResult> swapchainResults;

                {
                    VkSwapchainKHR swapchain2;
                    uint32_t index;
                    swapchain.presentData(&swapchain2, &index);

                    swapchains.emplace_back(swapchain2);
                    swapchainIndices.emplace_back(index);
                    swapchainResults.emplace_back(VK_SUCCESS);
                }

                VkPresentInfoKHR presentInfo{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &frameData[frameIndex].renderComplete,
                    .swapchainCount = static_cast<uint32_t>(swapchains.size()),
                    .pSwapchains = swapchains.data(),
                    .pImageIndices = swapchainIndices.data(),
                    .pResults = swapchainResults.data(),
                };

                VkResult res =
                    vkQueuePresentKHR(pGfxEnvironment->pQueueFamilies[0].queue[0], &presentInfo);
                if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                    // The associated window has been resized, will be fixed for the next frame
                    res = VK_SUCCESS;
                } else if (res != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }
        }
    SKIP_FRAME_RENDER:;
    }
SHUTDOWN_PROGRAM:
    FOE_LOG(General, Info, "Exiting main loop")

    if (pGfxEnvironment->device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(pGfxEnvironment->device);

    for (auto &it : frameData) {
        it.destroy(pGfxEnvironment->device);
    }

    for (auto &it : swapImageFramebuffers)
        vkDestroyFramebuffer(pGfxEnvironment->device, it, nullptr);

    cameraDescriptorPool.deinitialize();

    pipelinePool.deinitialize();
    shaderPool.deinitialize();
    builtinDescriptorSets.deinitialize(pGfxEnvironment->device);
    descriptorSetLayoutPool.deinitialize();

    renderPassPool.deinitialize();

    swapchain.destroy(pGfxEnvironment->device);
    destroyVkWindowData(&vkWindow, pGfxEnvironment->instance, pGfxEnvironment->device);

    foeDestroyWindow();

    foeGfxDestroyResourceUploader(&resUploader);

#ifdef EDITOR_MODE
    imguiRenderer.deinitialize(pGfxEnvironment);
#endif

    foeGfxDestroyEnvironment(pGfxEnvironment);

    return 0;
}