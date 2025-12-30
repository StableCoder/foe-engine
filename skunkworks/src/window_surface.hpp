// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOW_SURFACE_HPP
#define WINDOW_SURFACE_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.h>

#include <mutex>

struct WindowSurfaceData {
    std::mutex sync;
    // how many frames are in light to be rendered
    uint32_t inFlight{0};
    // whether or not to actively render this surface
    bool active{true};

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSampleCountFlags sampleCount{VK_SAMPLE_COUNT_1_BIT};

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;

    foeGfxVkSwapchain swapchain{FOE_NULL_HANDLE};
    bool acquiredImage{false};
    foeGfxVkSwapchainImageData acquiredImageData{FOE_NULL_HANDLE};

    foeGfxRenderTarget gfxOffscreenRenderTarget{FOE_NULL_HANDLE};
};

foeResultSet rebuildSurfaceSwapchain(WindowSurfaceData *pSurfaceData,
                                     foeGfxSession gfxSession,
                                     foeGfxDelayedCaller gfxDelayedCaller,
                                     bool vsync,
                                     uint32_t width,
                                     uint32_t height,
                                     VkFormat depthFormat);

#endif // WINDOW_SURFACE_HPP