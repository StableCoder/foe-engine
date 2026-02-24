// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WSI_QT_WINDOW_HPP
#define WSI_QT_WINDOW_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.h>
#include <foe/result.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <atomic>

#include "qt_vulkan_window.hpp"

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../window_surface.hpp"

struct Qt_WindowData {
    // sdl specific
    foeQtVulkanWindow *pWindow{nullptr};
    std::string title;
    bool close{false};
    bool resized{false};
    bool visible{false};
    bool vsync{false};

    // common
    KeyboardInput keyboard;
    MouseInput mouse;

    std::unique_ptr<std::atomic_bool> pNeedSwapchainRebuild;
    uint32_t desiredSampleCount{1};
    VkSampleCountFlags sampleCount;

    WindowSurfaceData renderSurfaceData;

    glm::vec3 position;
    glm::quat orientation;
    float fovY, nearZ, farZ;
    foeGfxRenderView renderView{FOE_NULL_HANDLE};
};

bool getQtVkExtensions(uint32_t *pCount, char const *const **ppExtensionNames);

foeResult createQtWindowVkSurface(foeGfxRuntime gfxRuntime,
                                  foeGfxSession gfxSession,
                                  Qt_WindowData *pWindowData,
                                  VkAllocationCallbacks *pAllocator,
                                  VkSurfaceKHR *pSurface);

void processQtEvents(uint32_t count, Qt_WindowData **ppWindowData);

void processQtUserInput(Qt_WindowData *pWindowData, double timeElapsedInSec);

#endif // WSI_QT_WINDOW_HPP