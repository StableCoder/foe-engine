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

#include "window.hpp"

#include <foe/graphics/vk/render_target.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include <memory>

auto performWindowMaintenance(WindowData *pWindow,
                              foeGfxSession gfxSession,
                              foeGfxDelayedDestructor gfxDelayedDestructor,
                              VkSampleCountFlags sampleCount,
                              VkFormat depthFormat) -> std::error_code {
    VkResult vkRes{VK_SUCCESS};
    std::error_code errC;

    // Check if need to rebuild a swapchain
    if (!pWindow->swapchain || pWindow->swapchain.needRebuild()) {
        int width, height;
        foeWsiWindowGetSize(pWindow->window, &width, &height);

        if (!pWindow->swapchain) {
            // Surface Format
            uint32_t formatCount;
            vkRes = vkGetPhysicalDeviceSurfaceFormatsKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                         pWindow->surface, &formatCount, nullptr);
            if (vkRes != VK_SUCCESS)
                return vkRes;

            std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats(
                new VkSurfaceFormatKHR[formatCount]);

            vkRes = vkGetPhysicalDeviceSurfaceFormatsKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                         pWindow->surface, &formatCount,
                                                         surfaceFormats.get());
            if (vkRes != VK_SUCCESS)
                return vkRes;

            pWindow->swapchain.surfaceFormat(surfaceFormats.get()[0]);

            // Present Mode
            uint32_t modeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount, nullptr);

            std::unique_ptr<VkPresentModeKHR[]> presentModes(new VkPresentModeKHR[modeCount]);

            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount,
                                                      presentModes.get());

            pWindow->swapchain.presentMode(presentModes.get()[0]);

            // Offscreen render target
            std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
                foeGfxVkRenderTargetSpec{
                    .format = pWindow->swapchain.surfaceFormat().format,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .count = 3,
                },
                foeGfxVkRenderTargetSpec{
                    .format = depthFormat,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .count = 3,
                },
            };

            errC = foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedDestructor,
                                              offscreenSpecs.data(), offscreenSpecs.size(),
                                              sampleCount, &pWindow->gfxOffscreenRenderTarget);
            if (errC) {
                return errC;
            }
        }

        // Create new swapchain
        foeGfxVkSwapchain newSwapchain;
        vkRes = newSwapchain.create(
            foeGfxVkGetPhysicalDevice(gfxSession), foeGfxVkGetDevice(gfxSession), pWindow->surface,
            pWindow->swapchain.surfaceFormat(), pWindow->swapchain.presentMode(),
            VK_IMAGE_USAGE_TRANSFER_DST_BIT, pWindow->swapchain, 3, width, height);
        if (vkRes != VK_SUCCESS)
            return vkRes;

        // If the old swapchain exists, we need to destroy it
        if (pWindow->swapchain) {
            foeGfxVkSwapchain swapchainCopy = pWindow->swapchain;
            foeGfxAddDelayedDestructionCall(gfxDelayedDestructor, [=](foeGfxSession session) {
                const_cast<foeGfxVkSwapchain &>(swapchainCopy).destroy(foeGfxVkGetDevice(session));
            });
        }

        pWindow->swapchain = newSwapchain;

        foeGfxUpdateRenderTargetExtent(pWindow->gfxOffscreenRenderTarget, width, height);
    }

    return errC;
}
