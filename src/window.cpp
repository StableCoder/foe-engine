// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/session.h>

#include "result.h"
#include "vk_result.h"

#include <memory>

namespace {

void destroy_foeGfxVkSwapchain(foeGfxVkSwapchain pSwapchain, foeGfxSession session) {
    foeGfxVkDestroySwapchain(session, pSwapchain);
}

} // namespace

foeResultSet performWindowMaintenance(WindowData *pWindow,
                                      foeGfxSession gfxSession,
                                      foeGfxDelayedCaller gfxDelayedDestructor,
                                      VkSampleCountFlags sampleCount,
                                      VkFormat depthFormat) {
    VkResult vkResult{VK_SUCCESS};
    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};

    // Check if need to rebuild a swapchain
    if (pWindow->swapchain == FOE_NULL_HANDLE || pWindow->needSwapchainRebuild) {
        pWindow->needSwapchainRebuild = false;

        int width, height;
        foeWsiWindowGetSize(pWindow->window, &width, &height);

        if (!pWindow->swapchain) {
            // Surface Format
            uint32_t formatCount;
            vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
                foeGfxVkGetPhysicalDevice(gfxSession), pWindow->surface, &formatCount, nullptr);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats(
                new VkSurfaceFormatKHR[formatCount]);

            vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                            pWindow->surface, &formatCount,
                                                            surfaceFormats.get());
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            pWindow->surfaceFormat = surfaceFormats[0];

            // Present Mode
            uint32_t modeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount, nullptr);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            std::unique_ptr<VkPresentModeKHR[]> presentModes(new VkPresentModeKHR[modeCount]);
            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount,
                                                      presentModes.get());
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            pWindow->surfacePresentMode = presentModes[0];

            // Offscreen render target
            std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
                foeGfxVkRenderTargetSpec{
                    .format = pWindow->surfaceFormat.format,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .count = 3,
                },
                foeGfxVkRenderTargetSpec{
                    .format = depthFormat,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .count = 3,
                },
            };

            result = foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedDestructor,
                                                offscreenSpecs.data(), offscreenSpecs.size(),
                                                sampleCount, &pWindow->gfxOffscreenRenderTarget);
            if (result.value != FOE_SUCCESS) {
                return result;
            }
        }

        // Create new swapchain
        foeGfxVkSwapchain newSwapchain = FOE_NULL_HANDLE;

        result = foeGfxVkCreateSwapchain(
            gfxSession, pWindow->surface, pWindow->surfaceFormat, pWindow->surfacePresentMode,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT, pWindow->swapchain, 3, width, height, &newSwapchain);
        if (result.value != FOE_SUCCESS)
            return result;

        // If the old swapchain exists, we need to destroy it
        if (pWindow->swapchain) {
            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_foeGfxVkSwapchain,
                                        (void *)pWindow->swapchain);
        }

        pWindow->swapchain = newSwapchain;

        VkExtent2D swapchainExtent = foeGfxVkGetSwapchainExtent(pWindow->swapchain);
        foeGfxUpdateRenderTargetExtent(pWindow->gfxOffscreenRenderTarget, swapchainExtent.width,
                                       swapchainExtent.height);
    }

    return result;
}
