/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <foe/error_code.h>
#include <foe/graphics/delayed_destructor.hpp>
#include <foe/graphics/render_target.hpp>
#include <foe/graphics/session.hpp>
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

foeResult performWindowMaintenance(WindowData *pWindow,
                                   foeGfxSession gfxSession,
                                   foeGfxDelayedDestructor gfxDelayedDestructor,
                                   VkSampleCountFlags sampleCount,
                                   VkFormat depthFormat);

#endif // WINDOW_HPP