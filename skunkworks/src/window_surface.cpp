// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window_surface.hpp"

#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/quaternion_math.hpp>

#include "vk_result.h"

#include <array>
#include <vector>

namespace {

void destroy_foeGfxVkSwapchain(foeGfxVkSwapchain pSwapchain, foeGfxSession session) {
    foeGfxVkDestroySwapchain(session, pSwapchain);
}

} // namespace

foeResultSet rebuildSurfaceSwapchain(WindowSurfaceData *pSurfaceData,
                                     foeGfxSession gfxSession,
                                     foeGfxDelayedCaller gfxDelayedCaller,
                                     bool vsync,
                                     uint32_t width,
                                     uint32_t height,
                                     VkFormat depthFormat) {
    VkResult vkResult;
    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};

    // Check if need to rebuild a swapchain
    if (!pSurfaceData->swapchain) {
        // Surface Format
        uint32_t formatCount;
        vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
            foeGfxVkGetPhysicalDevice(gfxSession), pSurfaceData->surface, &formatCount, nullptr);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        std::vector<VkSurfaceFormatKHR> surfaceFormats{formatCount, VkSurfaceFormatKHR{}};

        vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                        pSurfaceData->surface, &formatCount,
                                                        surfaceFormats.data());
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        pSurfaceData->surfaceFormat = surfaceFormats[0];

        // Present Mode
        uint32_t modeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                  pSurfaceData->surface, &modeCount, nullptr);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        std::vector<VkPresentModeKHR> presentModes{modeCount, VkPresentModeKHR{}};
        vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                  pSurfaceData->surface, &modeCount,
                                                  presentModes.data());
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // FIFO is always supported at a minimum
        pSurfaceData->presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!vsync) {
            // if not set for vsync, use any other presentation mode available
            for (auto mode : presentModes) {
                if (mode != VK_PRESENT_MODE_FIFO_KHR) {
                    pSurfaceData->presentMode = mode;
                    break;
                }
            }
        }

        // Offscreen render target
        std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
            foeGfxVkRenderTargetSpec{
                .format = pSurfaceData->surfaceFormat.format,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .count = 3,
            },
            foeGfxVkRenderTargetSpec{
                .format = depthFormat,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .count = 3,
            },
        };

        result = foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedCaller, offscreenSpecs.data(),
                                            offscreenSpecs.size(), pSurfaceData->sampleCount,
                                            &pSurfaceData->gfxOffscreenRenderTarget);
        if (result.value != FOE_SUCCESS) {
            return result;
        }
    }

    // Determine the minimum swapchain size
    VkSurfaceCapabilitiesKHR capabilities;
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                         pSurfaceData->surface, &capabilities);
    if (vkResult != VK_SUCCESS)
        return vk_to_foeResult(vkResult);

    // Create new swapchain
    foeGfxVkSwapchain newSwapchain = FOE_NULL_HANDLE;

    result = foeGfxVkCreateSwapchain(gfxSession, pSurfaceData->surface, pSurfaceData->surfaceFormat,
                                     pSurfaceData->presentMode, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                     pSurfaceData->swapchain, capabilities.minImageCount, width,
                                     height, &newSwapchain);
    if (result.value != FOE_SUCCESS)
        return result;

    // If the old swapchain exists, we need to destroy it
    if (pSurfaceData->swapchain) {
        foeGfxAddDefaultDelayedCall(gfxDelayedCaller,
                                    (PFN_foeGfxDelayedCall)destroy_foeGfxVkSwapchain,
                                    (void *)pSurfaceData->swapchain);
    }

    pSurfaceData->swapchain = newSwapchain;

    VkExtent2D swapchainExtent = foeGfxVkGetSwapchainExtent(pSurfaceData->swapchain);
    foeGfxUpdateRenderTargetExtent(pSurfaceData->gfxOffscreenRenderTarget, swapchainExtent.width,
                                   swapchainExtent.height);

    return result;
}