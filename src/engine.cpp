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
#include <foe/graphics/environment.hpp>
#include <foe/graphics/render_pass_pool.hpp>
#include <foe/graphics/swapchain.hpp>
#include <foe/log.hpp>
#include <foe/wsi.hpp>
#include <foe/wsi_vulkan.hpp>
#include <vk_error_code.hpp>

#include <array>
#include <chrono>
#include <iostream>

#include "frame_timer.hpp"
#include "stdout_sink.hpp"

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

    VkWindowData vkWindow;
    foeSwapchain swapchain;

    uint32_t frameIndex = 0;
    std::array<PerFrameData, 3> frameData;

    foeRenderPassPool renderPassPool;

    std::vector<VkFramebuffer> swapImageFramebuffers;
    bool swapchainRebuilt = false;

    auto res = foeGfxCreateEnvironment(true, "FoE Engine", 0, &pGfxEnvironment);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    if (!foeCreateWindow(1280, 720, "FoE Engine"))
        END_PROGRAM

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


    FOE_LOG(General, Info, "Entering main loop")
    while (!foeWindowGetShouldClose()) {
        static auto nextDisplayTime = std::chrono::steady_clock::now() + 1s;
        if (std::chrono::steady_clock::now() > nextDisplayTime) {
            FOE_LOG(General, Verbose, "Frame time: {}ms, FPS: {}",
                    frameTime.averageFrameTime<std::chrono::milliseconds>().count(),
                    frameTime.framesPerSecond())
            nextDisplayTime = std::chrono::steady_clock::now() + 1s;
        }

        swapchainRebuilt = false;
        foeWindowEventProcessing();

        auto *pMouse = foeGetMouse();
        auto *pKeyboard = foeGetKeyboard();

        if (foeWindowResized()) {
            // Swapchins will need rebuilding
            swapchain.requestRebuild();
        }

        // Vulkan Render Section
        uint32_t nextFrameIndex = (frameIndex + 1) % frameData.size();
        if (VK_SUCCESS == vkWaitForFences(pGfxEnvironment->device, 1,
                                          &frameData[nextFrameIndex].frameComplete, VK_TRUE, 0)) {
            frameTime.newFrame();
            frameIndex = nextFrameIndex;
            vkResetFences(pGfxEnvironment->device, 1, &frameData[frameIndex].frameComplete);

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

                        std::unique_ptr<VkSurfaceFormatKHR> surfaceFormats(
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

                        std::unique_ptr<VkPresentModeKHR> presentModes(
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
            VkResult res = swapchain.acquireNextImage(pGfxEnvironment->device,
                                                      frameData[frameIndex].presentImageAcquired);
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

            if (swapchainRebuilt) {
                for (auto &it : swapImageFramebuffers)
                    vkDestroyFramebuffer(pGfxEnvironment->device, it, nullptr);
                swapImageFramebuffers.clear();

                int width, height;
                foeWindowGetSize(&width, &height);
                VkImageView view;
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = swapImageRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = &view,
                    .width = (uint32_t)width,
                    .height = (uint32_t)height,
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
                    int width, height;
                    foeWindowGetSize(&width, &height);
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
                                .extent = {(uint32_t)width, (uint32_t)height},
                            },
                        .clearValueCount = 1,
                        .pClearValues = &clear,
                    };

                    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

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


    renderPassPool.deinitialize();

    swapchain.destroy(pGfxEnvironment->device);
    destroyVkWindowData(&vkWindow, pGfxEnvironment->instance, pGfxEnvironment->device);

    foeDestroyWindow();

    foeGfxDestroyEnvironment(pGfxEnvironment);

    return 0;
}