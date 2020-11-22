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

#ifndef FOE_GRAPHICS_SWAPCHAIN_HPP
#define FOE_GRAPHICS_SWAPCHAIN_HPP

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <vector>

/// Wrapper for the VkSwapchainKHR, performing all related functions to it
class foeSwapchain {
  public:
    FOE_GFX_EXPORT operator bool() const noexcept;
    FOE_GFX_EXPORT bool operator!() const noexcept;

    FOE_GFX_EXPORT operator VkSwapchainKHR() const noexcept;

    FOE_GFX_EXPORT VkResult create(VkPhysicalDevice physicalDevice,
                                   VkDevice device,
                                   VkSurfaceKHR surface,
                                   VkSurfaceFormatKHR surfaceFormat,
                                   VkPresentModeKHR presentMode,
                                   VkSwapchainKHR oldSwapchain,
                                   uint32_t chainSize,
                                   uint32_t width,
                                   uint32_t height);

    FOE_GFX_EXPORT void destroy(VkDevice device) noexcept;

    FOE_GFX_EXPORT VkResult acquireNextImage(VkDevice device, VkSemaphore imageReady) noexcept;
    FOE_GFX_EXPORT void presentData(VkSwapchainKHR *pSwapchain, uint32_t *pIndex) noexcept;

    FOE_GFX_EXPORT bool needRebuild() const noexcept;
    FOE_GFX_EXPORT void requestRebuild() noexcept;

    FOE_GFX_EXPORT VkSurfaceFormatKHR surfaceFormat() const noexcept;
    FOE_GFX_EXPORT void surfaceFormat(VkSurfaceFormatKHR surfaceFormat) noexcept;

    FOE_GFX_EXPORT VkPresentModeKHR presentMode() const noexcept;
    FOE_GFX_EXPORT void presentMode(VkPresentModeKHR presentMode) noexcept;

    FOE_GFX_EXPORT VkExtent2D extent() const noexcept;
    FOE_GFX_EXPORT uint32_t acquiredIndex() const noexcept;
    FOE_GFX_EXPORT uint32_t chainSize() const noexcept;
    FOE_GFX_EXPORT VkImageView imageView(uint32_t index) const noexcept;

  private:
    VkResult createSwapchainViews(VkDevice device);

    bool mNeedRebuild{false};
    VkSurfaceFormatKHR mSurfaceFormat{};
    VkPresentModeKHR mPresentMode{};

    VkSwapchainKHR mSwapchain{VK_NULL_HANDLE};

    VkExtent2D mExtent{};
    uint32_t mAcquiredIndex{UINT32_MAX};
    std::vector<VkImageView> mViews{};
};

#endif // FOE_GRAPHICS_SWAPCHAIN_HPP