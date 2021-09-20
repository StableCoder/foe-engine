/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/wsi/loader.h>
#include <foe/wsi/vulkan.h>
#include <foe/wsi/window.h>

#include <foe/plugin.h>

namespace {

typedef void (*PFN_foeWsiGlobalProcessing)();

typedef foeErrorCode (*PFN_foeWsiCreateWindow)(int, int, const char *, bool, foeWsiWindow *);
typedef void (*PFN_foeWsiDestroyWindow)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowProcessing)(foeWsiWindow);

typedef char const *(*PFN_foeWsiWindowGetTitle)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowSetTitle)(foeWsiWindow, char const *);

typedef bool (*PFN_foeWsiWindowGetShouldClose)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowSetShouldClose)(foeWsiWindow, bool);

typedef void (*PFN_foeWsiWindowGetSize)(foeWsiWindow, int *, int *);
typedef bool (*PFN_foeWsiWindowResized)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowResize)(foeWsiWindow, int, int);

typedef bool (*PFN_foeWsiWindowVisible)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowHide)(foeWsiWindow);
typedef void (*PFN_foeWsiWindowShow)(foeWsiWindow);

typedef void (*PFN_foeWsiWindowGetContentScale)(foeWsiWindow, float *, float *);

typedef foeWsiKeyboard const *(*PFN_foeWsiGetKeyboard)(foeWsiWindow);
typedef foeWsiMouse const *(*PFN_foeWsiGetMouse)(foeWsiWindow);

typedef foeErrorCode (*PFN_foeWsiWindowGetVulkanExtensions)(uint32_t *pExtensionCount,
                                                            char const ***pppExtensions);
typedef foeErrorCode (*PFN_foeWsiWindowGetVkSurface)(foeWsiWindow window,
                                                     VkInstance instance,
                                                     VkSurfaceKHR *pSurface);

struct DispatchTable {
    // Global
    PFN_foeWsiGlobalProcessing GlobalProcessing;
    // Window
    PFN_foeWsiCreateWindow CreateWindow;
    PFN_foeWsiDestroyWindow DestroyWindow;
    PFN_foeWsiWindowProcessing WindowProcessing;
    PFN_foeWsiWindowGetTitle WindowGetTitle;
    PFN_foeWsiWindowSetTitle WindowSetTitle;
    PFN_foeWsiWindowGetShouldClose WindowGetShouldClose;
    PFN_foeWsiWindowSetShouldClose WindowSetShouldClose;
    PFN_foeWsiWindowGetSize WindowGetSize;
    PFN_foeWsiWindowResized WindowResized;
    PFN_foeWsiWindowResize WindowResize;
    PFN_foeWsiWindowVisible WindowVisible;
    PFN_foeWsiWindowHide WindowHide;
    PFN_foeWsiWindowShow WindowShow;
    PFN_foeWsiWindowGetContentScale WindowGetContentScale;
    PFN_foeWsiGetKeyboard GetKeyboard;
    PFN_foeWsiGetMouse GetMouse;
    // Vulkan
    PFN_foeWsiWindowGetVulkanExtensions WindowGetVulkanExtensions;
    PFN_foeWsiWindowGetVkSurface WindowGetVkSurface;
};

DispatchTable gDispatchTable{};
foePlugin gWsiImplementation{FOE_NULL_HANDLE};

} // namespace

bool foeWsiLoadedImplementation() { return gWsiImplementation != FOE_NULL_HANDLE; }

void foeWsiLoadImplementation(char const *pPath) {
    if (gWsiImplementation == FOE_NULL_HANDLE) {
        foeCreatePlugin(pPath, &gWsiImplementation);

        if (gWsiImplementation != FOE_NULL_HANDLE) {
            gDispatchTable.GlobalProcessing = reinterpret_cast<PFN_foeWsiGlobalProcessing>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiGlobalProcessing"));

            gDispatchTable.CreateWindow = reinterpret_cast<PFN_foeWsiCreateWindow>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiCreateWindow"));

            gDispatchTable.DestroyWindow = reinterpret_cast<PFN_foeWsiDestroyWindow>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiDestroyWindow"));

            gDispatchTable.WindowProcessing = reinterpret_cast<PFN_foeWsiWindowProcessing>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowProcessing"));

            gDispatchTable.WindowGetTitle = reinterpret_cast<PFN_foeWsiWindowGetTitle>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetTitle"));

            gDispatchTable.WindowSetTitle = reinterpret_cast<PFN_foeWsiWindowSetTitle>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowSetTitle"));

            gDispatchTable.WindowGetShouldClose = reinterpret_cast<PFN_foeWsiWindowGetShouldClose>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetShouldClose"));

            gDispatchTable.WindowSetShouldClose = reinterpret_cast<PFN_foeWsiWindowSetShouldClose>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowSetShouldClose"));

            gDispatchTable.WindowGetSize = reinterpret_cast<PFN_foeWsiWindowGetSize>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetSize"));

            gDispatchTable.WindowResized = reinterpret_cast<PFN_foeWsiWindowResized>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowResized"));

            gDispatchTable.WindowResize = reinterpret_cast<PFN_foeWsiWindowResize>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowResize"));

            gDispatchTable.WindowVisible = reinterpret_cast<PFN_foeWsiWindowVisible>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowVisible"));

            gDispatchTable.WindowHide = reinterpret_cast<PFN_foeWsiWindowHide>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowHide"));

            gDispatchTable.WindowShow = reinterpret_cast<PFN_foeWsiWindowShow>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowShow"));

            gDispatchTable.WindowGetContentScale =
                reinterpret_cast<PFN_foeWsiWindowGetContentScale>(
                    foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetContentScale"));

            gDispatchTable.GetKeyboard = reinterpret_cast<PFN_foeWsiGetKeyboard>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiGetKeyboard"));

            gDispatchTable.GetMouse = reinterpret_cast<PFN_foeWsiGetMouse>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiGetMouse"));

            // Vulkan
            gDispatchTable.WindowGetVulkanExtensions =
                reinterpret_cast<PFN_foeWsiWindowGetVulkanExtensions>(
                    foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetVulkanExtensions"));

            gDispatchTable.WindowGetVkSurface = reinterpret_cast<PFN_foeWsiWindowGetVkSurface>(
                foeGetPluginSymbol(gWsiImplementation, "foeWsiWindowGetVkSurface"));
        }
    }
}

void foeWsiGlobalProcessing() { gDispatchTable.GlobalProcessing(); }

foeErrorCode foeWsiCreateWindow(
    int width, int height, const char *pTitle, bool visible, foeWsiWindow *pWindow) {
    return gDispatchTable.CreateWindow(width, height, pTitle, visible, pWindow);
}

void foeWsiDestroyWindow(foeWsiWindow window) { gDispatchTable.DestroyWindow(window); }

void foeWsiWindowProcessing(foeWsiWindow window) { gDispatchTable.WindowProcessing(window); }

char const *foeWsiWindowGetTitle(foeWsiWindow window) {
    return gDispatchTable.WindowGetTitle(window);
}

void foeWsiWindowSetTitle(foeWsiWindow window, const char *pTitle) {
    gDispatchTable.WindowSetTitle(window, pTitle);
}

bool foeWsiWindowGetShouldClose(foeWsiWindow window) {
    return gDispatchTable.WindowGetShouldClose(window);
}

void foeWsiWindowSetShouldClose(foeWsiWindow window, bool terminate) {
    gDispatchTable.WindowSetShouldClose(window, terminate);
}

void foeWsiWindowGetSize(foeWsiWindow window, int *pWidth, int *pHeight) {
    gDispatchTable.WindowGetSize(window, pWidth, pHeight);
}

bool foeWsiWindowResized(foeWsiWindow window) { return gDispatchTable.WindowResized(window); }

void foeWsiWindowResize(foeWsiWindow window, int width, int height) {
    gDispatchTable.WindowResize(window, width, height);
}

bool foeWsiWindowVisible(foeWsiWindow window) { return gDispatchTable.WindowVisible(window); }

void foeWsiWindowHide(foeWsiWindow window) { gDispatchTable.WindowHide(window); }

void foeWsiWindowShow(foeWsiWindow window) { gDispatchTable.WindowShow(window); }

void foeWsiWindowGetContentScale(foeWsiWindow window, float *pScaleX, float *pScaleY) {
    gDispatchTable.WindowGetContentScale(window, pScaleX, pScaleY);
}

foeWsiKeyboard const *foeWsiGetKeyboard(foeWsiWindow window) {
    return gDispatchTable.GetKeyboard(window);
}

foeWsiMouse const *foeWsiGetMouse(foeWsiWindow window) { return gDispatchTable.GetMouse(window); }

foeErrorCode foeWsiWindowGetVulkanExtensions(uint32_t *pExtensionCount,
                                             char const ***pppExtensions) {
    return gDispatchTable.WindowGetVulkanExtensions(pExtensionCount, pppExtensions);
}

foeErrorCode foeWsiWindowGetVkSurface(foeWsiWindow window,
                                      VkInstance instance,
                                      VkSurfaceKHR *pSurface) {
    return gDispatchTable.WindowGetVkSurface(window, instance, pSurface);
}