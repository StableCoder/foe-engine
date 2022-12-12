// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_SWAPCHAIN_HPP
#define FOE_GRAPHICS_VK_SWAPCHAIN_HPP

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <vector>

/// Wrapper for the VkSwapchainKHR, performing all related functions to it
class foeGfxVkSwapchain {
  public:
    FOE_GFX_EXPORT operator bool() const noexcept;
    FOE_GFX_EXPORT bool operator!() const noexcept;

    FOE_GFX_EXPORT operator VkSwapchainKHR() const noexcept;

    /**
     * @brief Creates a new swapchain
     * @param physicalDevice Vulkan physical device handle
     * @param device Vulkan logical device handle
     * @param surface Vulkan surface handle the swapchain presents on
     * @param surfaceFormat Chosen format to use on the swapchain
     * @param presentMode Chosed mode to present swapchain images
     * @param extraUsage By default all swapchains have the VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
     * usage, however more can be specified here.
     * @param oldSwapchain If the created swapchain is taking over from another one, specify it here
     * @param chainSize Desired number of swapchain images
     * @param width Desired width of the swapchain images
     * @param height Desired height of the swapchain images
     * @return VK_SUCCESS on success, an appropriate error enum otherwise
     */
    FOE_GFX_EXPORT VkResult create(VkPhysicalDevice physicalDevice,
                                   VkDevice device,
                                   VkSurfaceKHR surface,
                                   VkSurfaceFormatKHR surfaceFormat,
                                   VkPresentModeKHR presentMode,
                                   VkImageUsageFlags extraUsage,
                                   VkSwapchainKHR oldSwapchain,
                                   uint32_t chainSize,
                                   uint32_t width,
                                   uint32_t height);

    FOE_GFX_EXPORT void destroy(VkDevice device) noexcept;

    FOE_GFX_EXPORT VkResult acquireNextImage(VkDevice device) noexcept;
    FOE_GFX_EXPORT void presentData(VkSwapchainKHR *pSwapchain, uint32_t *pIndex) noexcept;

    FOE_GFX_EXPORT VkExtent2D extent() const noexcept;
    FOE_GFX_EXPORT uint32_t acquiredIndex() const noexcept;
    FOE_GFX_EXPORT uint32_t chainSize() const noexcept;
    FOE_GFX_EXPORT VkImage image(uint32_t index) const noexcept;
    FOE_GFX_EXPORT VkImageView imageView(uint32_t index) const noexcept;

    FOE_GFX_EXPORT VkSemaphore imageReadySemaphore() const noexcept;

  private:
    VkResult createSwapchainViews(VkDevice device, VkFormat format);
    VkResult createSemaphores(VkDevice device);

    VkSwapchainKHR mSwapchain{VK_NULL_HANDLE};

    VkExtent2D mExtent{};
    uint32_t mAcquiredIndex{UINT32_MAX};
    std::vector<VkImage> mImages{};
    std::vector<VkImageView> mViews{};
    std::vector<VkSemaphore> mSemaphores{};
    std::vector<VkSemaphore>::iterator mCurrentSemaphore;
};

#endif // FOE_GRAPHICS_VK_SWAPCHAIN_HPP