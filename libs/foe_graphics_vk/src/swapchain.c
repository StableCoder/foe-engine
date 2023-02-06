// Copyright (C) 2020-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/swapchain.h>

#include <foe/graphics/vk/session.h>

#include "result.h"
#include "vk_result.h"

#include <stdlib.h>

#define SEMAPHORE_COUNT(X) X * 2

struct foeGfxVkSwapchainImpl {
    VkSwapchainKHR swapchain;
    uint32_t chainSize;
    VkExtent2D extent;

    uint32_t currentSemaphoreIndex;
    VkSemaphore *pSemaphores;

    VkImage *pImages;
    VkImageView *pViews;
};

FOE_DEFINE_HANDLE_CASTS(swapchain, struct foeGfxVkSwapchainImpl, foeGfxVkSwapchain)

foeResultSet foeGfxVkCreateSwapchain(foeGfxSession session,
                                     VkSurfaceKHR surface,
                                     VkSurfaceFormatKHR surfaceFormat,
                                     VkPresentModeKHR presentMode,
                                     VkImageUsageFlags extraUsage,
                                     foeGfxVkSwapchain oldSwapchain,
                                     uint32_t minChainSize,
                                     uint32_t width,
                                     uint32_t height,
                                     foeGfxVkSwapchain *pSwapchain) {
    VkDevice const device = foeGfxVkGetDevice(session);

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        foeGfxVkGetPhysicalDevice(session), surface, &capabilities);
    if (vkResult != VK_SUCCESS)
        return vk_to_foeResult(vkResult);

    // Determine chain size
    if (minChainSize < capabilities.minImageCount)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_CHAIN_SIZE_LESS_THAN_SUPPORTED);
    if (capabilities.maxImageCount != 0 && minChainSize > capabilities.maxImageCount)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_CHAIN_SIZE_MORE_THAN_SUPPORTED);

    // Extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        // If the current extent is undefined, the size is set to the images requested
        extent.width = width;
        extent.height = height;
    } else {
        // If the surface size is defined, the swap chain must match
        extent = capabilities.currentExtent;
    }

    // Surface Transform
    VkSurfaceTransformFlagBitsKHR preTransform = {0};
    if ((capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0U) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = capabilities.currentTransform;
    }

    // Allocate main object
    struct foeGfxVkSwapchainImpl *pNewSwapchain = calloc(1, sizeof(struct foeGfxVkSwapchainImpl));
    if (pNewSwapchain == NULL)
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);

    // Set basics
    foeResultSet result = to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
    pNewSwapchain->extent = extent;

    // Create swapchain
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = minChainSize,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | extraUsage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = preTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
    };

    if (oldSwapchain != FOE_NULL_HANDLE) {
        // Old Swapchain?
        struct foeGfxVkSwapchainImpl *pOldSwapchain = swapchain_from_handle(oldSwapchain);
        swapchainCI.oldSwapchain = pOldSwapchain->swapchain;
    }

    vkResult = vkCreateSwapchainKHR(device, &swapchainCI, NULL, &pNewSwapchain->swapchain);
    if (vkResult != VK_SUCCESS) {
        result = vk_to_foeResult(vkResult);
        goto CREATE_FAILED;
    }

    { // Images
        uint32_t imageCount;
        vkResult = vkGetSwapchainImagesKHR(device, pNewSwapchain->swapchain, &imageCount, NULL);
        if (vkResult != VK_SUCCESS) {
            result = vk_to_foeResult(vkResult);
            goto CREATE_FAILED;
        }

        pNewSwapchain->chainSize = imageCount;

        pNewSwapchain->pImages = (VkImage *)malloc(imageCount * sizeof(VkImage));
        if (pNewSwapchain->pImages == NULL) {
            result = to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        vkResult = vkGetSwapchainImagesKHR(device, pNewSwapchain->swapchain, &imageCount,
                                           pNewSwapchain->pImages);
        if (vkResult != VK_SUCCESS) {
            result = vk_to_foeResult(vkResult);
            goto CREATE_FAILED;
        }
    }

    { // Image Views
        pNewSwapchain->pViews = calloc(pNewSwapchain->chainSize, sizeof(VkImageView));
        if (pNewSwapchain->pViews == NULL) {
            result = to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        VkImageViewCreateInfo viewCI = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,
            .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                           VK_COMPONENT_SWIZZLE_A},
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        for (uint32_t i = 0; i < pNewSwapchain->chainSize; ++i) {
            viewCI.image = pNewSwapchain->pImages[i];

            vkResult = vkCreateImageView(device, &viewCI, NULL, pNewSwapchain->pViews + i);
            if (vkResult != VK_SUCCESS) {
                result = vk_to_foeResult(vkResult);
                goto CREATE_FAILED;
            }
        }
    }

    { // Semaphores
        pNewSwapchain->pSemaphores =
            (VkSemaphore *)calloc(SEMAPHORE_COUNT(pNewSwapchain->chainSize), sizeof(VkSemaphore));
        if (pNewSwapchain->pSemaphores == NULL) {
            result = to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        pNewSwapchain->currentSemaphoreIndex = 0;

        VkSemaphoreCreateInfo semaphoreCI = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        for (uint32_t i = 0; i < SEMAPHORE_COUNT(pNewSwapchain->chainSize); ++i) {
            vkResult =
                vkCreateSemaphore(device, &semaphoreCI, NULL, pNewSwapchain->pSemaphores + i);
            if (vkResult != VK_SUCCESS) {
                result = vk_to_foeResult(vkResult);
                goto CREATE_FAILED;
            }
        }
    }

CREATE_FAILED:
    if (result.value == FOE_SUCCESS) {
        *pSwapchain = swapchain_to_handle(pNewSwapchain);
    } else {
        foeGfxVkDestroySwapchain(session, swapchain_to_handle(pNewSwapchain));
    }

    return result;
}

void foeGfxVkDestroySwapchain(foeGfxSession session, foeGfxVkSwapchain swapchain) {
    VkDevice const device = foeGfxVkGetDevice(session);
    struct foeGfxVkSwapchainImpl *pSwapchain = swapchain_from_handle(swapchain);

    // Semaphores
    if (pSwapchain->pSemaphores != NULL) {
        for (uint32_t i = 0; i < SEMAPHORE_COUNT(pSwapchain->chainSize); ++i) {
            if (pSwapchain->pSemaphores[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(device, pSwapchain->pSemaphores[i], NULL);
        }

        free(pSwapchain->pSemaphores);
    }

    // Image Views
    if (pSwapchain->pViews != NULL) {
        for (uint32_t i = 0; i < pSwapchain->chainSize; ++i) {
            if (pSwapchain->pViews[i] != VK_NULL_HANDLE)
                vkDestroyImageView(device, pSwapchain->pViews[i], NULL);
        }

        free(pSwapchain->pViews);
    }

    // Images
    if (pSwapchain->pImages != NULL)
        free(pSwapchain->pImages);

    // Swapchain
    if (pSwapchain->swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device, pSwapchain->swapchain, NULL);

    free(pSwapchain);
}

VkExtent2D foeGfxVkGetSwapchainExtent(foeGfxVkSwapchain swapchain) {
    struct foeGfxVkSwapchainImpl *pSwapchain = swapchain_from_handle(swapchain);

    return pSwapchain->extent;
}

VkResult foeGfxVkAcquireSwapchainImage(foeGfxSession session,
                                       foeGfxVkSwapchain swapchain,
                                       foeGfxVkSwapchainImageData *pSwapchainImageData) {
    struct foeGfxVkSwapchainImpl *pSwapchain = swapchain_from_handle(swapchain);

    uint32_t nextSemaphoreIndex = pSwapchain->currentSemaphoreIndex + 1;
    if (nextSemaphoreIndex >= pSwapchain->currentSemaphoreIndex)
        nextSemaphoreIndex = 0;

    uint32_t swapchainImageIndex;
    VkResult vkResult = vkAcquireNextImageKHR(foeGfxVkGetDevice(session), pSwapchain->swapchain, 0,
                                              pSwapchain->pSemaphores[nextSemaphoreIndex],
                                              VK_NULL_HANDLE, &swapchainImageIndex);
    if (vkResult == VK_SUCCESS || vkResult == VK_SUBOPTIMAL_KHR) {
        foeGfxVkSwapchainImageData outgoingData = {
            .extent = pSwapchain->extent,
            .readySemaphore = pSwapchain->pSemaphores[nextSemaphoreIndex],
            .image = pSwapchain->pImages[swapchainImageIndex],
            .view = pSwapchain->pViews[swapchainImageIndex],
            .swapchain = pSwapchain->swapchain,
            .imageIndex = swapchainImageIndex,
        };

        *pSwapchainImageData = outgoingData;

        pSwapchain->currentSemaphoreIndex = nextSemaphoreIndex;
    }

    return vkResult;
}