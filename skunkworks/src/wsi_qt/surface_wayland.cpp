// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <foe/graphics/vk/runtime.h>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan_wayland.h>

VkSurfaceKHR createSurfaceWayland(foeGfxRuntime gfxRuntime, QWindow *pWindow) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(gfxRuntime);

    PFN_vkCreateWaylandSurfaceKHR pfnCreateSurface =
        reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(
            vkGetInstanceProcAddr(vkInstance, "vkCreateWaylandSurfaceKHR"));
    if (!pfnCreateSurface)
        std::abort();

    QPlatformNativeInterface *pNativeInterface = QGuiApplication::platformNativeInterface();
    wl_display *wlDisplay =
        (wl_display *)pNativeInterface->nativeResourceForWindow("display", pWindow);
    struct wl_surface *wlSurface =
        (struct wl_surface *)pNativeInterface->nativeResourceForWindow("surface", pWindow);

    VkWaylandSurfaceCreateInfoKHR surfaceCI{
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = wlDisplay,
        .surface = wlSurface,
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult vkResult = pfnCreateSurface(vkInstance, &surfaceCI, nullptr, &surface);

    return surface;
}