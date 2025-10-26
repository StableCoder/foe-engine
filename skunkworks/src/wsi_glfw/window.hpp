// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WSI_GLFW_WINDOW_HPP
#define WSI_GLFW_WINDOW_HPP

#include <GLFW/glfw3.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/session.h>
#include <foe/graphics/vk/swapchain.h>
#include <foe/result.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../frame_timer.hpp"
#include "../hid/keyboard.hpp"
#include "../hid/mouse.hpp"

struct GLFW_WindowData {
    GLFWwindow *pWindow{FOE_NULL_HANDLE};
    bool resized{false};

    KeyboardInput keyboard;
    MouseInput mouse;

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

bool createGlfwWindow(int width,
                      int height,
                      char const *pTitle,
                      bool visible,
                      GLFWwindow **ppWindow,
                      MouseInput *pMouse,
                      KeyboardInput *pKeyboard,
                      bool *pResized);

void destroyGlfwWindow(GLFW_WindowData *pWindow);

foeResultSet performGlfwWindowMaintenance(GLFW_WindowData *pWindow,
                                          foeGfxSession gfxSession,
                                          foeGfxDelayedCaller gfxDelayedDestructor,
                                          VkSampleCountFlags sampleCount,
                                          VkFormat depthFormat);

#endif // WSI_GLFW_WINDOW_HPP