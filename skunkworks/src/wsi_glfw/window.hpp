// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WSI_GLFW_WINDOW_HPP
#define WSI_GLFW_WINDOW_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.h>
#include <foe/result.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"
#include "../window_surface.hpp"

struct GLFWwindow;

struct GLFW_WindowData {
    GLFWwindow *pWindow{nullptr};
    bool resized{false};
    bool vsync{false};
    bool requestClose{false};

    KeyboardInput keyboard;
    MouseInput mouse;

    bool needSwapchainRebuild{false};
    uint32_t desiredSampleCount{1};
    VkSampleCountFlags sampleCount;

    WindowSurfaceData renderSurfaceData;

    glm::vec3 position;
    glm::quat orientation;
    float fovY, nearZ, farZ;
    foeGfxRenderView renderView{FOE_NULL_HANDLE};
};

bool createGlfwWindow(int width, int height, char const *pTitle, GLFW_WindowData *pWindowData);

void destroyGlfwWindow(foeGfxRuntime gfxRuntime,
                       foeGfxSession gfxSession,
                       GLFW_WindowData *pWindow);

void processGlfwEvents(uint32_t count, GLFW_WindowData **ppWindowData);

void processUserInput(GLFW_WindowData *pWindowData, double timeElapsedInSec);

void getGlfwWindowLogicalSize(GLFW_WindowData *pWindowData, int *pWidth, int *pHeight);

void getGlfwWindowPixelSize(GLFW_WindowData *pWindowData, int *pWidth, int *pHeight);

void getGlfwWindowScale(GLFW_WindowData *pWindowData, float *xScale, float *yScale);

bool getGlfwVkInstanceExtensions(uint32_t *pCount, char const *const **ppExtensionNames);

foeResult createGlfwWindowVkSurface(foeGfxRuntime gfxRuntime,
                                    // optional, will be used to check compatability if not NULL
                                    foeGfxSession gfxSession,
                                    GLFW_WindowData *pWindowData,
                                    VkAllocationCallbacks *pAllocator,
                                    VkSurfaceKHR *pSurface);

#endif // WSI_GLFW_WINDOW_HPP