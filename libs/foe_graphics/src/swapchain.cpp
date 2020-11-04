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

#include <foe/graphics/swapchain.hpp>

#include <memory>

foeSwapchain::operator bool() const noexcept { return mSwapchain != VK_NULL_HANDLE; }
bool foeSwapchain::operator!() const noexcept { return mSwapchain == VK_NULL_HANDLE; }

foeSwapchain::operator VkSwapchainKHR() const noexcept { return mSwapchain; }

VkResult foeSwapchain::create(VkPhysicalDevice physicalDevice,
                              VkDevice device,
                              VkSurfaceKHR surface,
                              VkSurfaceFormatKHR surfaceFormat,
                              VkPresentModeKHR presentMode,
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
        extent = capabilities.currentExtent;
    }

    // Surface Transform
    VkSurfaceTransformFlagBitsKHR preTransform = {};
    if ((capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0U) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = capabilities.currentTransform;
    }

    mSurfaceFormat = surfaceFormat;
    mPresentMode = presentMode;

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 3,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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

    // Get the new image views
    res = createSwapchainViews(device);
    if (res != VK_SUCCESS) {
        destroy(device);
    }

    return res;
}

void foeSwapchain::destroy(VkDevice device) noexcept {
    for (auto &it : mViews)
        vkDestroyImageView(device, it, nullptr);
    mViews.clear();

    vkDestroySwapchainKHR(device, mSwapchain, nullptr);
    mSwapchain = VK_NULL_HANDLE;
}

VkResult foeSwapchain::acquireNextImage(VkDevice device, VkSemaphore imageReady) noexcept {
    if (mAcquiredIndex != UINT32_MAX) {
        // If the current image hasn't been 'presented', don't try to acquire another yet
        return VK_EVENT_SET;
    }

    uint32_t tempIndex;
    auto res = vkAcquireNextImageKHR(device, mSwapchain, 0, imageReady, VK_NULL_HANDLE, &tempIndex);
    if (res == VK_SUCCESS)
        mAcquiredIndex = tempIndex;

    return res;
}

void foeSwapchain::presentData(VkSwapchainKHR *pSwapchain, uint32_t *pIndex) noexcept {
    if (mAcquiredIndex != UINT32_MAX) {
        *pSwapchain = mSwapchain;
        *pIndex = mAcquiredIndex;
        mAcquiredIndex = UINT32_MAX;
    }
}

bool foeSwapchain::needRebuild() const noexcept { return mNeedRebuild; }

void foeSwapchain::requestRebuild() noexcept { mNeedRebuild = true; }

VkSurfaceFormatKHR foeSwapchain::surfaceFormat() const noexcept { return mSurfaceFormat; }

void foeSwapchain::surfaceFormat(VkSurfaceFormatKHR surfaceFormat) noexcept {
    mSurfaceFormat = surfaceFormat;
    mNeedRebuild = true;
}

VkPresentModeKHR foeSwapchain::presentMode() const noexcept { return mPresentMode; }

void foeSwapchain::presentMode(VkPresentModeKHR presentMode) noexcept {
    mPresentMode = presentMode;
    mNeedRebuild = true;
}

uint32_t foeSwapchain::acquiredIndex() const noexcept { return mAcquiredIndex; }

uint32_t foeSwapchain::chainSize() const noexcept { return mViews.size(); }

VkImageView foeSwapchain::imageView(uint32_t index) const noexcept { return mViews[index]; }

VkResult foeSwapchain::createSwapchainViews(VkDevice device) {
    uint32_t imageCount;
    VkResult res = vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, nullptr);
    if (res != VK_SUCCESS)
        return res;

    std::unique_ptr<VkImage[]> images(new VkImage[imageCount]);
    res = vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, images.get());
    if (res != VK_SUCCESS)
        return res;

    mViews.clear();
    mViews.reserve(imageCount);

    VkImageViewCreateInfo viewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = mSurfaceFormat.format,
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
        viewCI.image = images.get()[i];

        VkImageView newView;
        res = vkCreateImageView(device, &viewCI, nullptr, &newView);
        if (res != VK_SUCCESS)
            return res;
        mViews.emplace_back(newView);
    }

    return res;
}