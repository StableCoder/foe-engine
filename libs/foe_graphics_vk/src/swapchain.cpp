// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/swapchain.hpp>

#include <memory>

foeGfxVkSwapchain::operator bool() const noexcept { return mSwapchain != VK_NULL_HANDLE; }
bool foeGfxVkSwapchain::operator!() const noexcept { return mSwapchain == VK_NULL_HANDLE; }

foeGfxVkSwapchain::operator VkSwapchainKHR() const noexcept { return mSwapchain; }

VkResult foeGfxVkSwapchain::create(VkPhysicalDevice physicalDevice,
                                   VkDevice device,
                                   VkSurfaceKHR surface,
                                   VkSurfaceFormatKHR surfaceFormat,
                                   VkPresentModeKHR presentMode,
                                   VkImageUsageFlags extraUsage,
                                   VkSwapchainKHR oldSwapchain,
                                   uint32_t chainSize,
                                   uint32_t width,
                                   uint32_t height) {
    if (mSwapchain != VK_NULL_HANDLE)
        return VK_ERROR_INITIALIZATION_FAILED;

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult res =
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    if (res != VK_SUCCESS)
        return res;

    // Determine chain size
    if (chainSize < capabilities.minImageCount) {
        chainSize = capabilities.minImageCount;
    }
    if (capabilities.maxImageCount > 0 && chainSize > capabilities.maxImageCount) {
        chainSize = capabilities.maxImageCount;
    }

    // Extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        // If the current extent is undefined, the size is set to the images requested
        extent = VkExtent2D{width, height};
    } else {
        // If the surface size is defined, the swap chain must match
#ifndef __APPLE__
        extent = capabilities.currentExtent;
#else
        // For whatever reason, GLFW is scaling the surface size in the window on 'retina' screens,
        // so a 1280x720 window on a retina screen has a surface size 2x that, leading to the
        // rendered portion of the window being a quarter of the screen.
        extent = VkExtent2D{width, height};
#endif
    }

    // Surface Transform
    VkSurfaceTransformFlagBitsKHR preTransform = {};
    if ((capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0U) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = capabilities.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 3,
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
        .oldSwapchain = oldSwapchain,
    };

    res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &mSwapchain);
    if (res != VK_SUCCESS)
        return res;

    mExtent = extent;

    // Get the new image views
    res = createSwapchainViews(device, surfaceFormat.format);
    if (res != VK_SUCCESS)
        goto CREATE_FAILED;

    res = createSemaphores(device);
    if (res != VK_SUCCESS)
        goto CREATE_FAILED;

CREATE_FAILED:
    if (res != VK_SUCCESS)
        destroy(device);

    return res;
}

void foeGfxVkSwapchain::destroy(VkDevice device) noexcept {
    for (auto &it : mSemaphores) {
        if (it != VK_NULL_HANDLE)
            vkDestroySemaphore(device, it, nullptr);
    }
    mSemaphores.clear();

    for (auto &it : mViews) {
        if (it != VK_NULL_HANDLE)
            vkDestroyImageView(device, it, nullptr);
    }
    mViews.clear();

    mImages.clear();

    vkDestroySwapchainKHR(device, mSwapchain, nullptr);
    mSwapchain = VK_NULL_HANDLE;
}

VkResult foeGfxVkSwapchain::acquireNextImage(VkDevice device) noexcept {
    if (mAcquiredIndex != UINT32_MAX) {
        // If the current image hasn't been 'presented', don't try to acquire another yet
        return VK_EVENT_SET;
    }

    auto newSemaphoreIt = mCurrentSemaphore + 1;
    if (newSemaphoreIt >= mSemaphores.end()) {
        newSemaphoreIt = mSemaphores.begin();
    }
    uint32_t tempIndex;
    auto res =
        vkAcquireNextImageKHR(device, mSwapchain, 0, *newSemaphoreIt, VK_NULL_HANDLE, &tempIndex);
    if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR) {
        mAcquiredIndex = tempIndex;
        mCurrentSemaphore = newSemaphoreIt;
    }

    return res;
}

void foeGfxVkSwapchain::presentData(VkSwapchainKHR *pSwapchain, uint32_t *pIndex) noexcept {
    if (mAcquiredIndex != UINT32_MAX) {
        *pSwapchain = mSwapchain;
        *pIndex = mAcquiredIndex;
        mAcquiredIndex = UINT32_MAX;
    }
}

VkExtent2D foeGfxVkSwapchain::extent() const noexcept { return mExtent; }

uint32_t foeGfxVkSwapchain::acquiredIndex() const noexcept { return mAcquiredIndex; }

uint32_t foeGfxVkSwapchain::chainSize() const noexcept {
    return static_cast<uint32_t>(mViews.size());
}

VkImage foeGfxVkSwapchain::image(uint32_t index) const noexcept { return mImages[index]; }

VkImageView foeGfxVkSwapchain::imageView(uint32_t index) const noexcept { return mViews[index]; }

VkSemaphore foeGfxVkSwapchain::imageReadySemaphore() const noexcept { return *mCurrentSemaphore; }

VkResult foeGfxVkSwapchain::createSwapchainViews(VkDevice device, VkFormat format) {
    uint32_t imageCount;
    VkResult res = vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, nullptr);
    if (res != VK_SUCCESS)
        return res;

    mImages.resize(imageCount);
    res = vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, mImages.data());
    if (res != VK_SUCCESS)
        return res;

    mViews.clear();
    mViews.reserve(imageCount);

    VkImageViewCreateInfo viewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                       VK_COMPONENT_SWIZZLE_A},
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    for (uint32_t i = 0; i < imageCount; ++i) {
        viewCI.image = mImages[i];

        VkImageView newView;
        res = vkCreateImageView(device, &viewCI, nullptr, &newView);
        if (res != VK_SUCCESS)
            return res;
        mViews.emplace_back(newView);
    }

    return res;
}

VkResult foeGfxVkSwapchain::createSemaphores(VkDevice device) {
    VkResult vkRes{VK_SUCCESS};
    mSemaphores = std::vector<VkSemaphore>(mImages.size(), VK_NULL_HANDLE);
    mCurrentSemaphore = mSemaphores.begin();

    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for (auto &it : mSemaphores) {
        vkRes = vkCreateSemaphore(device, &semaphoreCI, nullptr, &it);
        if (vkRes != VK_SUCCESS)
            goto SEMAPHORE_CREATE_FAILED;
    }

SEMAPHORE_CREATE_FAILED:
    return vkRes;
}