// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.h>
#include <foe/result.h>
#include <foe/wsi/window.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "frame_timer.hpp"

struct WindowData {
    foeWsiWindow window{FOE_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    foeGfxRenderTarget gfxOffscreenRenderTarget{FOE_NULL_HANDLE};

    bool needSwapchainRebuild{false};
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR surfacePresentMode;
    foeGfxVkSwapchain swapchain{FOE_NULL_HANDLE};
    bool acquiredImage{false};
    foeGfxVkSwapchainImageData acquiredImageData;

    glm::vec3 position;
    glm::quat orientation;
    float fovY, nearZ, farZ;
    foeGfxRenderView renderView{FOE_NULL_HANDLE};

    FrameTimer frameTime;
};

foeResultSet performWindowMaintenance(WindowData *pWindow,
                                      foeGfxSession gfxSession,
                                      foeGfxDelayedCaller gfxDelayedDestructor,
                                      VkSampleCountFlags sampleCount,
                                      VkFormat depthFormat);

#endif // WINDOW_HPP