// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <GLFW/glfw3.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/quaternion_math.hpp>

#include "../result.h"

namespace {

struct PrivateWindowData {
    MouseInput *pMouse;
    KeyboardInput *pKeyboard;
    bool *pResized;
    bool *pClose;
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

void closeCallback(GLFWwindow *pWindow) {
    auto *pWindowData = static_cast<PrivateWindowData *>(glfwGetWindowUserPointer(pWindow));

    *pWindowData->pClose = true;
}

} // namespace

bool createGlfwWindow(int width, int height, char const *pTitle, GLFW_WindowData *pWindowData) {
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow *pNewWindow = glfwCreateWindow(width, height, pTitle, nullptr, nullptr);
    if (!pNewWindow)
        return false;

    bool result = true;

    PrivateWindowData *pNewPrivateWindowData = new (std::nothrow) PrivateWindowData{
        .pMouse = &pWindowData->mouse,
        .pKeyboard = &pWindowData->keyboard,
        .pResized = &pWindowData->resized,
        .pClose = &pWindowData->requestClose,
    };
    if (!pNewPrivateWindowData) {
        glfwDestroyWindow(pNewWindow);
        return false;
    }

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

    // Misc Callbacks
    glfwSetWindowSizeCallback(pNewWindow, windowResizedCallback);
    glfwSetWindowCloseCallback(pNewWindow, closeCallback);

    pWindowData->pWindow = pNewWindow;

    return true;
}

void destroyGlfwWindow(foeGfxRuntime gfxRuntime,
                       foeGfxSession gfxSession,
                       GLFW_WindowData *pWindow) {
    // graphics items
    if (pWindow->renderSurfaceData.gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
        foeGfxDestroyRenderTarget(pWindow->renderSurfaceData.gfxOffscreenRenderTarget);

    if (pWindow->renderSurfaceData.swapchain != FOE_NULL_HANDLE)
        foeGfxVkDestroySwapchain(gfxSession, pWindow->renderSurfaceData.swapchain);

    if (pWindow->renderSurfaceData.surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime),
                            pWindow->renderSurfaceData.surface, nullptr);

    // glfw items
    PrivateWindowData *pPrivateWindowData =
        (PrivateWindowData *)glfwGetWindowUserPointer(pWindow->pWindow);
    if (pPrivateWindowData)
        delete pPrivateWindowData;

    glfwDestroyWindow(pWindow->pWindow);
}

void processGlfwEvents(uint32_t count, GLFW_WindowData **ppWindowData) {
    for (uint32_t i = 0; i < count; ++i) {
        GLFW_WindowData *it = ppWindowData[i];

        it->mouse.preprocessing();
        it->keyboard.preprocessing();
        it->resized = false;
    }

    glfwPollEvents();
}

void processUserInput(GLFW_WindowData *pWindowData, double timeElapsedInSec) {
    constexpr float movementMultiplier = 10.f;
    constexpr float rorationMultiplier = 40.f;

    MouseInput *pMouse = &pWindowData->mouse;
    KeyboardInput *pKeyboard = &pWindowData->keyboard;

    float multiplier = timeElapsedInSec * 3.f; // 3 units per second

    if (pMouse->inWindow) {
        if (pKeyboard->keycodeDown(GLFW_KEY_Z)) { // Up
            pWindowData->position +=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(GLFW_KEY_X)) { // Down
            pWindowData->position -=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(GLFW_KEY_W)) { // Forward
            pWindowData->position +=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(GLFW_KEY_S)) { // Back
            pWindowData->position -=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(GLFW_KEY_A)) { // Left
            pWindowData->position +=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(GLFW_KEY_D)) { // Right
            pWindowData->position -=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pMouse->buttonDown(GLFW_MOUSE_BUTTON_1)) {
            pWindowData->orientation =
                changeYaw(pWindowData->orientation,
                          -glm::radians(pMouse->oldPosition.x - pMouse->position.x));
            pWindowData->orientation = changePitch(
                pWindowData->orientation, glm::radians(pMouse->oldPosition.y - pMouse->position.y));

            if (pKeyboard->keycodeDown(GLFW_KEY_Q)) { // Roll Left
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, glm::radians(rorationMultiplier * multiplier));
            }
            if (pKeyboard->keycodeDown(GLFW_KEY_E)) { // Roll Right
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, -glm::radians(rorationMultiplier * multiplier));
            }
        }
    }
}

void getGlfwWindowLogicalSize(GLFW_WindowData *pWindowData, int *pWidth, int *pHeight) {
    glfwGetWindowSize(pWindowData->pWindow, pWidth, pHeight);
}

void getGlfwWindowPixelSize(GLFW_WindowData *pWindowData, int *pWidth, int *pHeight) {
    glfwGetFramebufferSize(pWindowData->pWindow, pWidth, pHeight);
}

void getGlfwWindowScale(GLFW_WindowData *pWindowData, float *xScale, float *yScale) {
    glfwGetWindowContentScale(pWindowData->pWindow, xScale, yScale);
}

bool getGlfwVkInstanceExtensions(uint32_t *pCount, char const *const **pppExtensionNames) {
    if (!glfwVulkanSupported())
        return false;

    *pppExtensionNames = glfwGetRequiredInstanceExtensions(pCount);

    return true;
}

foeResult createGlfwWindowVkSurface(foeGfxRuntime gfxRuntime,
                                    foeGfxSession gfxSession,
                                    GLFW_WindowData *pWindowData,
                                    VkAllocationCallbacks *pAllocator,
                                    VkSurfaceKHR *pSurface) {
    VkSurfaceKHR newSurface;
    VkResult vkResult = glfwCreateWindowSurface(foeGfxVkGetRuntimeInstance(gfxRuntime),
                                                pWindowData->pWindow, pAllocator, &newSurface);
    if (vkResult != VK_SUCCESS)
        return (foeResult)FOE_SKUNKWORKS_ERROR_UNABLE_TO_CREATE_SURFACE;

    if (gfxSession != FOE_NULL_HANDLE) {
        // graphics session already exists, so we want to check that the surface we have is
        // compatible with it before proceeding
        VkBool32 usableSurface;
        vkGetPhysicalDeviceSurfaceSupportKHR(foeGfxVkGetPhysicalDevice(gfxSession), 0, newSurface,
                                             &usableSurface);

        if (usableSurface == VK_FALSE) {
            // it is not compatible, so destroy the surface and return saying as much
            vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime), newSurface, pAllocator);

            return (foeResult)FOE_SKUNKWORKS_ERROR_INCOMPATIBLE_GRAPHICS_SESSION;
        }
    }

SURFACE_CREATE_SUCCESS:
    *pSurface = newSurface;
    return (foeResult)FOE_SKUNKWORKS_SUCCESS;
}
