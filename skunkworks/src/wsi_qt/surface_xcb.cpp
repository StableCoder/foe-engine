// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QWindow>
#include <foe/graphics/vk/runtime.h>
#include <vulkan/vulkan.h>

// xcb
#include <xcb/xcb.h>

#include <vulkan/vulkan_xcb.h>

VkSurfaceKHR createSurfaceXcb(foeGfxRuntime gfxRuntime,
                              QGuiApplication const *pGuiApplication,
                              QWindow *pWindow) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(gfxRuntime);

    PFN_vkCreateXcbSurfaceKHR pfnCreateSurface = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(
        vkGetInstanceProcAddr(vkInstance, "vkCreateXcbSurfaceKHR"));
    if (!pfnCreateSurface)
        std::abort();

    QNativeInterface::QX11Application *pNativeApplication =
        pGuiApplication->nativeInterface<QNativeInterface::QX11Application>();

    VkXcbSurfaceCreateInfoKHR surfaceCI{
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = (xcb_connection_t *)pNativeApplication->connection(),
        .window = (xcb_window_t)pWindow->winId(),
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult vkResult = pfnCreateSurface(vkInstance, &surfaceCI, nullptr, &surface);

    return surface;
}