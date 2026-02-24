// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QWindow>
#include <foe/graphics/vk/runtime.h>
#include <vulkan/vulkan.h>

// win32
#include <windows.h>

#include <vulkan/vulkan_win32.h>

VkSurfaceKHR createSurfaceWin32(foeGfxRuntime gfxRuntime, QWindow *pWindow) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(gfxRuntime);

    PFN_vkCreateWin32SurfaceKHR pfnCreateSurface = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
        vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR"));
    if (!pfnCreateSurface)
        std::abort();

    HWND windowHandle = (HWND)pWindow->winId();

    VkWin32SurfaceCreateInfoKHR surfaceCI{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd = windowHandle,
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult vkResult = pfnCreateSurface(vkInstance, &surfaceCI, nullptr, &surface);

    return surface;
}