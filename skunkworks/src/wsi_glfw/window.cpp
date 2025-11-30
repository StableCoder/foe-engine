// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <GLFW/glfw3.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/session.h>

#include "../result.h"
#include "../vk_result.h"

#include <vector>

namespace {

struct PrivateWindowData {
    MouseInput *pMouse;
    KeyboardInput *pKeyboard;
    bool *pResized;
};

void keyCallback(GLFWwindow *pWindow, int keycode, int scancode, int action, int mods) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pKeyboard = pWindowData->pKeyboard;

    if (action == GLFW_PRESS) {
        pKeyboard->pressedCodes.emplace_back(keycode, scancode);
        pKeyboard->downCodes.emplace_back(keycode, scancode);
    } else if (action == GLFW_REPEAT) {
        pKeyboard->repeatCode = {
            .keycode = (uint32_t)keycode,
            .scancode = (uint32_t)scancode,
        };
    } else if (action == GLFW_RELEASE) {
        pKeyboard->releasedCodes.emplace_back(keycode, scancode);

        // remove the code pair form the set of held-down codes
        bool codeFound = false;
        auto const endIt = pKeyboard->downCodes.end();
        for (auto it = pKeyboard->downCodes.begin(); it != endIt; ++it) {
            if (it->keycode == keycode && it->scancode == scancode) {
                pKeyboard->downCodes.erase(it);
                codeFound = true;
                break;
            }
        }

        // if the exact same code pair could not be found, meaning it wasn't entered previously,
        // then something is very wrong and needs to be fixed
        assert(codeFound);
    }
}

void charCallback(GLFWwindow *pWindow, unsigned int codepoint) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pKeyboard = pWindowData->pKeyboard;

    pKeyboard->unicodeChar = codepoint;
}

void positionCallback(GLFWwindow *pWindow, double xPos, double yPos) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pMouse = pWindowData->pMouse;

    pMouse->position = {xPos, yPos};
}

void cursorPositionCallback(GLFWwindow *pWindow, double xPos, double yPos) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pMouse = pWindowData->pMouse;

    pMouse->position = {xPos, yPos};
}

void cursorEnterCallback(GLFWwindow *pWindow, int entered) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pMouse = pWindowData->pMouse;

    pMouse->inWindow = entered;
}

void scrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pMouse = pWindowData->pMouse;

    pMouse->scroll = {xOffset, yOffset};
}

void buttonCallback(GLFWwindow *pWindow, int button, int action, int) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));
    auto *pMouse = pWindowData->pMouse;

    if (action == GLFW_PRESS) {
        pMouse->pressedButtons.insert(button);
        pMouse->downButtons.insert(button);
    } else if (action == GLFW_RELEASE) {
        pMouse->releasedButtons.insert(button);
        pMouse->downButtons.erase(button);
    }
}

void windowResizedCallback(GLFWwindow *pWindow, int, int) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));

    *pWindowData->pResized = true;
}

} // namespace

bool createGlfwWindow(int width,
                      int height,
                      char const *pTitle,
                      bool visible,
                      GLFWwindow **ppWindow,
                      MouseInput *pMouse,
                      KeyboardInput *pKeyboard,
                      bool *pResized) {
    if (!glfwInit())
        std::abort();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (visible) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    GLFWwindow *pNewWindow = glfwCreateWindow(width, height, pTitle, nullptr, nullptr);
    if (!pNewWindow)
        std::abort();

    bool result = true;

    PrivateWindowData *pNewPrivateWindowData = new (std::nothrow) PrivateWindowData{
        .pMouse = pMouse,
        .pKeyboard = pKeyboard,
        .pResized = pResized,
    };
    if (!pNewPrivateWindowData)
        result = false;

    // Set User Data Pointer
    glfwSetWindowUserPointer(pNewWindow, pNewPrivateWindowData);

    // Keyboard Callbacks
    glfwSetKeyCallback(pNewWindow, keyCallback);
    glfwSetCharCallback(pNewWindow, charCallback);

    // Mouse Callbacks
    glfwSetCursorPosCallback(pNewWindow, positionCallback);
    glfwSetCursorEnterCallback(pNewWindow, cursorEnterCallback);
    glfwSetScrollCallback(pNewWindow, scrollCallback);
    glfwSetMouseButtonCallback(pNewWindow, buttonCallback);

    // Window Callbacks
    glfwSetWindowSizeCallback(pNewWindow, windowResizedCallback);

    if (result) {
        *ppWindow = pNewWindow;
    } else {
        if (pNewPrivateWindowData)
            delete pNewPrivateWindowData;

        glfwDestroyWindow(pNewWindow);
    }

    return result;
}

void destroyGlfwWindow(GLFW_WindowData *pWindow) {
    PrivateWindowData *pPrivateWindowData =
        (PrivateWindowData *)glfwGetWindowUserPointer(pWindow->pWindow);
    if (pPrivateWindowData)
        delete pPrivateWindowData;

    glfwDestroyWindow(pWindow->pWindow);
}

namespace {

void destroy_foeGfxVkSwapchain(foeGfxVkSwapchain pSwapchain, foeGfxSession session) {
    foeGfxVkDestroySwapchain(session, pSwapchain);
}

} // namespace

foeResultSet performGlfwWindowMaintenance(GLFW_WindowData *pWindow,
                                          foeGfxSession gfxSession,
                                          foeGfxDelayedCaller gfxDelayedDestructor,
                                          VkSampleCountFlags sampleCount,
                                          VkFormat depthFormat) {
    VkResult vkResult{VK_SUCCESS};
    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};

    // Check if need to rebuild a swapchain
    if (pWindow->swapchain == FOE_NULL_HANDLE || pWindow->needSwapchainRebuild) {
        pWindow->needSwapchainRebuild = false;

        int width, height;
        glfwGetFramebufferSize(pWindow->pWindow, &width, &height);

        if (!pWindow->swapchain) {
            // Surface Format
            uint32_t formatCount;
            vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
                foeGfxVkGetPhysicalDevice(gfxSession), pWindow->surface, &formatCount, nullptr);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            std::vector<VkSurfaceFormatKHR> surfaceFormats{formatCount, VkSurfaceFormatKHR{}};

            vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                            pWindow->surface, &formatCount,
                                                            surfaceFormats.data());
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            pWindow->surfaceFormat = surfaceFormats[0];

            // Present Mode
            uint32_t modeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount, nullptr);
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            std::vector<VkPresentModeKHR> presentModes{modeCount, VkPresentModeKHR{}};
            vkGetPhysicalDeviceSurfacePresentModesKHR(foeGfxVkGetPhysicalDevice(gfxSession),
                                                      pWindow->surface, &modeCount,
                                                      presentModes.data());
            if (vkResult != VK_SUCCESS)
                return vk_to_foeResult(vkResult);

            pWindow->surfacePresentMode = presentModes[0];

            // Offscreen render target
            std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
                foeGfxVkRenderTargetSpec{
                    .format = pWindow->surfaceFormat.format,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .count = 3,
                },
                foeGfxVkRenderTargetSpec{
                    .format = depthFormat,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .count = 3,
                },
            };

            result = foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedDestructor,
                                                offscreenSpecs.data(), offscreenSpecs.size(),
                                                sampleCount, &pWindow->gfxOffscreenRenderTarget);
            if (result.value != FOE_SUCCESS) {
                return result;
            }
        }

        // Determine the minimum swapchain size
        VkSurfaceCapabilitiesKHR capabilities;
        VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            foeGfxVkGetPhysicalDevice(gfxSession), pWindow->surface, &capabilities);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        // Create new swapchain
        foeGfxVkSwapchain newSwapchain = FOE_NULL_HANDLE;

        result = foeGfxVkCreateSwapchain(gfxSession, pWindow->surface, pWindow->surfaceFormat,
                                         pWindow->surfacePresentMode,
                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT, pWindow->swapchain,
                                         capabilities.minImageCount, width, height, &newSwapchain);
        if (result.value != FOE_SUCCESS)
            return result;

        // If the old swapchain exists, we need to destroy it
        if (pWindow->swapchain) {
            foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                        (PFN_foeGfxDelayedCall)destroy_foeGfxVkSwapchain,
                                        (void *)pWindow->swapchain);
        }

        pWindow->swapchain = newSwapchain;

        VkExtent2D swapchainExtent = foeGfxVkGetSwapchainExtent(pWindow->swapchain);
        foeGfxUpdateRenderTargetExtent(pWindow->gfxOffscreenRenderTarget, swapchainExtent.width,
                                       swapchainExtent.height);
    }

    return result;
}
