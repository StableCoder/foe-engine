// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <foe/graphics/vk/runtime.h>
#include <vulkan/vulkan.h>

// xcb
#include <xcb/xcb.h>

#include <vulkan/vulkan_xcb.h>

VkSurfaceKHR createSurfaceXcb(foeGfxRuntime gfxRuntime, QWindow *pWindow) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(gfxRuntime);

    PFN_vkCreateXcbSurfaceKHR pfnCreateSurface = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(
        vkGetInstanceProcAddr(vkInstance, "vkCreateXcbSurfaceKHR"));
    if (!pfnCreateSurface)
        std::abort();

    QPlatformNativeInterface *pNativeInterface = QGuiApplication::platformNativeInterface();
    xcb_connection_t *xcbConnection =
        (xcb_connection_t *)pNativeInterface->nativeResourceForWindow("connection", pWindow);
    xcb_window_t xcbWindow = pWindow->winId();

    VkXcbSurfaceCreateInfoKHR surfaceCI{
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = xcbConnection,
        .window = xcbWindow,
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult vkResult = pfnCreateSurface(vkInstance, &surfaceCI, nullptr, &surface);

    return surface;
}