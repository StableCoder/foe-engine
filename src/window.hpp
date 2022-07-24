// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <foe/error_code.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.hpp>
#include <foe/wsi/window.h>

#include "frame_timer.hpp"

struct WindowData {
    foeWsiWindow window{FOE_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    foeGfxVkSwapchain swapchain;
    foeGfxRenderTarget gfxOffscreenRenderTarget{FOE_NULL_HANDLE};

    FrameTimer frameTime;
    // foeEntityId attachedCamera;
};

foeResultSet performWindowMaintenance(WindowData *pWindow,
                                      foeGfxSession gfxSession,
                                      foeGfxDelayedCaller gfxDelayedDestructor,
                                      VkSampleCountFlags sampleCount,
                                      VkFormat depthFormat);

#endif // WINDOW_HPP