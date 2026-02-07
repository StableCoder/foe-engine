// Copyright (C) 2025-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/quaternion_math.hpp>

#include "../result.h"

bool createSDL3Window(int width, int height, char const *pTitle, SDL3_WindowData *pWindowData) {
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;

    SDL_WindowFlags windowFlags =
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window *pNewWindow = SDL_CreateWindow(pTitle, width, height, windowFlags);

    if (!pNewWindow)
        return false;

    pWindowData->pWindow = pNewWindow;
    pWindowData->id = SDL_GetWindowID(pNewWindow);

    return true;
}

void destroySDL3Window(foeGfxRuntime gfxRuntime,
                       foeGfxSession gfxSession,
                       SDL3_WindowData *pWindow) {
    if (pWindow->renderSurfaceData.gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
        foeGfxDestroyRenderTarget(pWindow->renderSurfaceData.gfxOffscreenRenderTarget);

    if (pWindow->renderSurfaceData.swapchain != FOE_NULL_HANDLE)
        foeGfxVkDestroySwapchain(gfxSession, pWindow->renderSurfaceData.swapchain);

    if (pWindow->renderSurfaceData.surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime),
                            pWindow->renderSurfaceData.surface, nullptr);

    SDL_DestroyWindow(pWindow->pWindow);
}

namespace {

SDL3_WindowData *getWindowDataFromID(SDL_WindowID windowID,
                                     uint32_t count,
                                     SDL3_WindowData **ppWindowData) {
    for (uint32_t i = 0; i < count; ++i) {
        if (ppWindowData[i]->id == windowID)
            return ppWindowData[i];
    }

    return nullptr;
}

} // namespace

void processSDL3Events(uint32_t count, SDL3_WindowData **ppWindowData) {
    for (uint32_t i = 0; i < count; ++i) {
        SDL3_WindowData *it = ppWindowData[i];

        it->mouse.preprocessing();
        it->keyboard.preprocessing();
        it->resized = false;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        SDL3_WindowData *pWindowData;
        switch (event.type) {
        case SDL_EVENT_WINDOW_HIDDEN:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->visible = false;
            }
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->visible = true;
            }
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->close = true;
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->resized = true;
            }
            break;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.inWindow = true;
            }
            break;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            pWindowData = getWindowDataFromID(event.window.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.inWindow = false;
            }
            break;

        case SDL_EVENT_KEY_DOWN:
            pWindowData = getWindowDataFromID(event.key.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                if (event.key.repeat) {
                    pWindowData->keyboard.repeatCode = {
                        .keycode = event.key.key,
                        .scancode = event.key.raw,
                    };
                } else {
                    pWindowData->keyboard.pressedCodes.emplace_back(event.key.key, event.key.raw);
                    pWindowData->keyboard.downCodes.emplace_back(event.key.key, event.key.raw);
                }
            }
            break;

        case SDL_EVENT_KEY_UP:
            pWindowData = getWindowDataFromID(event.key.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->keyboard.releasedCodes.emplace_back(event.key.key, event.key.raw);

                // remove the code pair form the set of held-down codes
                bool codeFound = false;
                auto const endIt = pWindowData->keyboard.downCodes.end();
                for (auto it = pWindowData->keyboard.downCodes.begin(); it != endIt; ++it) {
                    if (it->keycode == event.key.key && it->scancode == event.key.raw) {
                        pWindowData->keyboard.downCodes.erase(it);
                        codeFound = true;
                        break;
                    }
                }

                // if the exact same code pair could not be found, meaning it wasn't entered
                // previously, then something is very wrong and needs to be fixed
                assert(codeFound);
            }
            break;

        case SDL_EVENT_TEXT_INPUT:
            pWindowData = getWindowDataFromID(event.text.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                // @TODO Do UTF-8 to unicode conversion
                // window.keyboard.unicodeChar = event.text.text;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            pWindowData = getWindowDataFromID(event.button.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.downButtons.insert(event.button.button);
                pWindowData->mouse.pressedButtons.insert(event.button.button);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            pWindowData = getWindowDataFromID(event.button.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.releasedButtons.insert(event.button.button);
                pWindowData->mouse.downButtons.erase(event.button.button);
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            pWindowData = getWindowDataFromID(event.motion.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.position.x = event.motion.x;
                pWindowData->mouse.position.y = event.motion.y;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            pWindowData = getWindowDataFromID(event.wheel.windowID, count, ppWindowData);
            if (pWindowData != nullptr) {
                pWindowData->mouse.scroll.x = event.wheel.x;
                pWindowData->mouse.scroll.y = event.wheel.y;
            }
            break;

        default:
            break;
        }
    }
}

void processSDL3UserInput(SDL3_WindowData *pWindowData, double timeElapsedInSec) {
    constexpr float movementMultiplier = 10.f;
    constexpr float rorationMultiplier = 40.f;

    MouseInput *const pMouse = &pWindowData->mouse;
    KeyboardInput *const pKeyboard = &pWindowData->keyboard;

    float multiplier = timeElapsedInSec * 3.f; // 3 units per second

    if (pMouse->inWindow) {
        if (pKeyboard->keycodeDown(SDLK_Z)) { // Up
            pWindowData->position +=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(SDLK_X)) { // Down
            pWindowData->position -=
                upVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(SDLK_W)) { // Forward
            pWindowData->position +=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(SDLK_S)) { // Back
            pWindowData->position -=
                forwardVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keycodeDown(SDLK_A)) { // Left
            pWindowData->position +=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keycodeDown(SDLK_D)) { // Right
            pWindowData->position -=
                leftVec(pWindowData->orientation) * movementMultiplier * multiplier;
        }

        if (pMouse->buttonDown(SDL_BUTTON_LEFT)) {
            pWindowData->orientation =
                changeYaw(pWindowData->orientation,
                          -glm::radians(pMouse->oldPosition.x - pMouse->position.x));
            pWindowData->orientation = changePitch(
                pWindowData->orientation, glm::radians(pMouse->oldPosition.y - pMouse->position.y));

            if (pKeyboard->keycodeDown(SDLK_Q)) { // Roll Left
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, glm::radians(rorationMultiplier * multiplier));
            }
            if (pKeyboard->keycodeDown(SDLK_E)) { // Roll Right
                pWindowData->orientation = changeRoll(
                    pWindowData->orientation, -glm::radians(rorationMultiplier * multiplier));
            }
        }
    }
}

void getSDL3WindowLogicalSize(SDL3_WindowData *pWindowData, int *pWidth, int *pHeight) {
    SDL_GetWindowSize(pWindowData->pWindow, pWidth, pHeight);
}

void getSDL3WindowPixelSize(SDL3_WindowData *pWindowData, int *pWidth, int *pHeight) {
    SDL_GetWindowSizeInPixels(pWindowData->pWindow, pWidth, pHeight);
}

void getSDL3WindowScale(SDL3_WindowData *pWindowData, float *xScale, float *yScale) {
    float scale = SDL_GetWindowDisplayScale(pWindowData->pWindow);

    *xScale = scale;
    *yScale = scale;
}

bool getSDL3VkExtensions(uint32_t *pCount, char const *const **ppExtensionNames) {
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;

    *ppExtensionNames = SDL_Vulkan_GetInstanceExtensions(pCount);
    return true;
}

foeResult createSDL3WindowVkSurface(foeGfxRuntime gfxRuntime,
                                    foeGfxSession gfxSession,
                                    SDL3_WindowData *pWindowData,
                                    VkAllocationCallbacks *pAllocator,
                                    VkSurfaceKHR *pSurface) {
    VkSurfaceKHR newSurface;
    bool created = SDL_Vulkan_CreateSurface(
        pWindowData->pWindow, foeGfxVkGetRuntimeInstance(gfxRuntime), nullptr, &newSurface);
    if (!created)
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

    *pSurface = newSurface;
    return (foeResult)FOE_SKUNKWORKS_SUCCESS;
}
