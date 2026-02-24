// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <QGuiApplication>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/quaternion_math.hpp>

#include "../result.h"

#if __APPLE__
// char const *cCocoaExtensions[] = {"VK_KHR_surface", "VK_EXT_metal_surface"};
#elif _WIN32
VkSurfaceKHR createSurfaceWin32(foeGfxRuntime gfxRuntime, QWindow *pWindow);

char const *cWin32Extensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
#elif __linux__
VkSurfaceKHR createSurfaceWayland(foeGfxRuntime gfxRuntime, QWindow *pWindow);

char const *cWaylandExtensions[] = {"VK_KHR_surface", "VK_KHR_wayland_surface"};

VkSurfaceKHR createSurfaceXcb(foeGfxRuntime gfxRuntime, QWindow *pWindow);

char const *cXcbExtensions[] = {"VK_KHR_surface", "VK_KHR_xcb_surface"};
#endif

bool getQtVkExtensions(uint32_t *pCount, char const *const **ppExtensionNames) {
    QString const platformName = QGuiApplication::platformName();

#if __APPLE__
    if (platformName == "cocoa") {
        // *pCount = 2;
        // if (ppExtensionNames)
        //     *ppExtensionNames = cCocoaExtensions;

        // return true;
    }
#elif _WIN32
    if (platformName == "windows") {
        *pCount = 2;
        if (ppExtensionNames)
            *ppExtensionNames = cWin32Extensions;

        return true;
    }
#elif __linux__
    if (platformName == "wayland") {
        *pCount = 2;
        if (ppExtensionNames)
            *ppExtensionNames = cWaylandExtensions;

        return true;

    } else if (platformName == "xcb") {
        *pCount = 2;
        if (ppExtensionNames)
            *ppExtensionNames = cXcbExtensions;

        return true;
    }
#endif

    return false;
}

foeResult createQtWindowVkSurface(foeGfxRuntime gfxRuntime,
                                  foeGfxSession gfxSession,
                                  Qt_WindowData *pWindowData,
                                  VkAllocationCallbacks *pAllocator,
                                  VkSurfaceKHR *pSurface) {
    VkSurfaceKHR newSurface = VK_NULL_HANDLE;
    QString const platformName = QGuiApplication::platformName();

#if __APPLE__
    if (platformName == "cocoa") {
    }
#elif _WIN32
    if (platformName == "windows") {
        newSurface = createSurfaceWin32(gfxRuntime, pWindowData->pWindow);
    }
#elif __linux__
    if (platformName == "wayland") {
        newSurface = createSurfaceWayland(gfxRuntime, pWindowData->pWindow);
    } else if (platformName == "xcb") {
        newSurface = createSurfaceXcb(gfxRuntime, pWindowData->pWindow);
    }
#endif

    if (newSurface == VK_NULL_HANDLE) {
        return (foeResult)FOE_SKUNKWORKS_ERROR_UNABLE_TO_CREATE_SURFACE;
    }

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

    *pSurface = newSurface;
    return (foeResult)FOE_SKUNKWORKS_SUCCESS;
}

void processQtEvents(uint32_t count, Qt_WindowData **ppWindowData) {
    for (uint32_t i = 0; i < count; ++i) {
        Qt_WindowData *pWindowData = ppWindowData[i];

        pWindowData->pWindow->processKeyboardEvents(&pWindowData->keyboard);
        pWindowData->pWindow->processMouseEvents(&pWindowData->mouse);
    }
}

void processQtUserInput(Qt_WindowData *pWindowData, double timeElapsedInSec) {
    constexpr float movementMultiplier = 10.f;
    constexpr float rorationMultiplier = 40.f;

    MouseInput *const pMouse = &pWindowData->mouse;
    KeyboardInput *const pKeyboard = &pWindowData->keyboard;

    float multiplier = timeElapsedInSec * 3.f; // 3 units per second

    if (pMouse->inWindow) {
        if (pKeyboard->keycodeDown(Qt::Key_Z)) { // Up
            pWindowData->position +=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(Qt::Key_X)) { // Down
            pWindowData->position -=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(Qt::Key_W)) { // Forward
            pWindowData->position +=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(Qt::Key_S)) { // Back
            pWindowData->position -=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(Qt::Key_A)) { // Left
            pWindowData->position +=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(Qt::Key_D)) { // Right
            pWindowData->position -=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pMouse->buttonDown(Qt::MouseButton::LeftButton)) {
            pWindowData->orientation =
                changeYaw(pWindowData->orientation,
                          -glm::radians(pMouse->oldPosition.x - pMouse->position.x));
            pWindowData->orientation = changePitch(
                pWindowData->orientation, glm::radians(pMouse->oldPosition.y - pMouse->position.y));

            if (pKeyboard->keycodeDown(Qt::Key_Q)) { // Roll Left
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, glm::radians(rorationMultiplier * multiplier));
            }
            if (pKeyboard->keycodeDown(Qt::Key_E)) { // Roll Right
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, -glm::radians(rorationMultiplier * multiplier));
            }
        }
    }
}