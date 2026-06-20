// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QWindow>
#include <foe/graphics/vk/runtime.h>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan_wayland.h>

VkSurfaceKHR createSurfaceWayland(foeGfxRuntime gfxRuntime,
                                  QGuiApplication const *pGuiApplication,
                                  QWindow *pWindow) {
    VkInstance vkInstance = foeGfxVkGetRuntimeInstance(gfxRuntime);

    PFN_vkCreateWaylandSurfaceKHR pfnCreateSurface =
        reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(
            vkGetInstanceProcAddr(vkInstance, "vkCreateWaylandSurfaceKHR"));
    if (!pfnCreateSurface)
        std::abort();

    QNativeInterface::QWaylandApplication *pNativeApplication =
        pGuiApplication->nativeInterface<QNativeInterface::QWaylandApplication>();

    VkWaylandSurfaceCreateInfoKHR surfaceCI{
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = (wl_display *)pNativeApplication->display(),
        .surface = (struct wl_surface *)pWindow->winId(),
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult vkResult = pfnCreateSurface(vkInstance, &surfaceCI, nullptr, &surface);

    return surface;
}