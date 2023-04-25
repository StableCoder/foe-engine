// Copyright (C) 2020-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_SWAPCHAIN_H
#define FOE_GRAPHICS_VK_SWAPCHAIN_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxVkSwapchain)

typedef struct foeGfxVkSwapchainImageData {
    // Image Data
    VkExtent2D extent;
    VkSemaphore readySemaphore;
    VkImage image;
    VkImageView view;

    // Swapchain/Presentation Data
    VkSwapchainKHR swapchain;
    uint32_t imageIndex;
} foeGfxVkSwapchainImageData;

FOE_GFX_EXPORT
foeResultSet foeGfxVkCreateSwapchain(foeGfxSession session,
                                     VkSurfaceKHR surface,
                                     VkSurfaceFormatKHR surfaceFormat,
                                     VkPresentModeKHR presentMode,
                                     VkImageUsageFlags extraUsage,
                                     foeGfxVkSwapchain oldSwapchain,
                                     uint32_t minChainSize,
                                     uint32_t width,
                                     uint32_t height,
                                     foeGfxVkSwapchain *pSwapchain);

FOE_GFX_EXPORT
void foeGfxVkDestroySwapchain(foeGfxSession session, foeGfxVkSwapchain swapchain);

FOE_GFX_EXPORT
VkExtent2D foeGfxVkGetSwapchainExtent(foeGfxVkSwapchain swapchain);

FOE_GFX_EXPORT
VkResult foeGfxVkAcquireSwapchainImage(foeGfxSession session,
                                       foeGfxVkSwapchain swapchain,
                                       foeGfxVkSwapchainImageData *pSwapchainImageData);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_SWAPCHAIN_H