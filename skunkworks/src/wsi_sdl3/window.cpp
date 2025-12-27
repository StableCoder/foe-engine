// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/quaternion_math.hpp>

#include "../vk_result.h"

#include <vector>

bool createSDL3Window(int width, int height, char const *pTitle, SDL3_WindowData *pWindowData) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::abort();
    }

    SDL_WindowFlags windowFlags =
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window *pNewWindow = SDL_CreateWindow(pTitle, width, height, windowFlags);

    if (!pNewWindow) {
        return false;
    }

    pWindowData->pWindow = pNewWindow;
    pWindowData->id = SDL_GetWindowID(pNewWindow);

    return true;
}

void destroySDL3Window(foeGfxRuntime gfxRuntime,
                       foeGfxSession gfxSession,
                       SDL3_WindowData *pWindow) {
    if (pWindow->gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
        foeGfxDestroyRenderTarget(pWindow->gfxOffscreenRenderTarget);

    if (pWindow->swapchain != FOE_NULL_HANDLE)
        foeGfxVkDestroySwapchain(gfxSession, pWindow->swapchain);

    if (pWindow->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime), pWindow->surface, nullptr);

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
                //  window.keyboard.unicodeChar = event.text.text;
                printf("Character: %s\n", event.text.text);
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

void getSDL3WindowScale(SDL3_WindowData *pWindowData, float *xScale, int *yScale) {
    float scale = SDL_GetWindowDisplayScale(pWindowData->pWindow);

    *xScale = scale;
    *yScale = scale;
}

bool getSDL3VkExtensions(uint32_t *pCount, char const *const **ppExtensionNames) {
    *ppExtensionNames = SDL_Vulkan_GetInstanceExtensions(pCount);
    return true;
}

bool createSDL3WindowVkSurface(foeGfxRuntime gfxRuntime,
                               SDL3_WindowData *pWindowData,
                               VkAllocationCallbacks *pAllocator,
                               VkSurfaceKHR *pSurface) {
    return SDL_Vulkan_CreateSurface(pWindowData->pWindow, foeGfxVkGetRuntimeInstance(gfxRuntime),
                                    nullptr, pSurface);
}

namespace {

void destroy_foeGfxVkSwapchain(foeGfxVkSwapchain pSwapchain, foeGfxSession session) {
    foeGfxVkDestroySwapchain(session, pSwapchain);
}

} // namespace

foeResultSet performSDL3WindowMaintenance(SDL3_WindowData *pWindow,
                                          foeGfxSession gfxSession,
                                          foeGfxDelayedCaller gfxDelayedDestructor,
                                          VkFormat depthFormat) {
    VkResult vkResult{VK_SUCCESS};
    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};

    // Check if need to rebuild a swapchain
    if (pWindow->swapchain == FOE_NULL_HANDLE || pWindow->needSwapchainRebuild) {
        pWindow->needSwapchainRebuild = false;

        int width, height;
        getSDL3WindowPixelSize(pWindow, &width, &height);

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

            // FIFO is always supported at a minimum
            pWindow->surfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;
            if (!pWindow->vsync) {
                // if not set for vsync, use any other presentation mode available
                for (auto mode : presentModes) {
                    if (mode != VK_PRESENT_MODE_FIFO_KHR) {
                        pWindow->surfacePresentMode = mode;
                        break;
                    }
                }
            }

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

            result = foeGfxVkCreateRenderTarget(
                gfxSession, gfxDelayedDestructor, offscreenSpecs.data(), offscreenSpecs.size(),
                pWindow->sampleCount, &pWindow->gfxOffscreenRenderTarget);
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