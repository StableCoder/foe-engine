// Copyright (C) 2025-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef WSI_SDL3_WINDOW_HPP
#define WSI_SDL3_WINDOW_HPP

#include <SDL3/SDL.h>
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

struct SDL3_WindowData {
    // sdl specific
    SDL_Window *pWindow{nullptr};
    SDL_WindowID id{0};
    bool close{false};
    bool resized{false};
    bool visible{false};
    bool vsync{false};

    // common
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

bool createSDL3Window(int width, int height, char const *pTitle, SDL3_WindowData *pWindowData);

void destroySDL3Window(foeGfxRuntime gfxRuntime,
                       foeGfxSession gfxSession,
                       SDL3_WindowData *pWindow);

void processSDL3Events(uint32_t count, SDL3_WindowData **ppWindowData);

void processSDL3UserInput(SDL3_WindowData *pWindowData, double timeElapsedInSec);

void getSDL3WindowLogicalSize(SDL3_WindowData *pWindowData, int *pWidth, int *pHeight);

void getSDL3WindowPixelSize(SDL3_WindowData *pWindowData, int *pWidth, int *pHeight);

void getSDL3WindowScale(SDL3_WindowData *pWindowData, float *xScale, float *yScale);

bool getSDL3VkExtensions(uint32_t *pCount, char const *const **ppExtensionNames);

foeResult createSDL3WindowVkSurface(foeGfxRuntime gfxRuntime,
                                    // optional, will be used to check compatability if not NULL
                                    foeGfxSession gfxSession,
                                    SDL3_WindowData *pWindowData,
                                    VkAllocationCallbacks *pAllocator,
                                    VkSurfaceKHR *pSurface);

#endif // WSI_SDL3_WINDOW_HPP