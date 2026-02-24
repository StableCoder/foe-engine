// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/upload_context.h>
#include <foe/result.h>
#include <foe/search_paths.hpp>
#include <foe/split_thread_pool.h>
#include <foe/xr/runtime.h>

#include "frame_timer.hpp"
#include "per_frame_data.hpp"
#include "settings.hpp"

// WSI
#ifdef FOE_SKUNKWORKS_GLFW
#include "wsi_glfw/window.hpp"
#endif
#ifdef FOE_SKUNKWORKS_SDL3
#include "wsi_sdl3/window.hpp"
#endif
#ifdef FOE_SKUNKWORKS_QT
#include "wsi_qt/window.hpp"
#endif

#include <array>
#include <vector>

#ifdef FOE_XR_SUPPORT
#include "xr.hpp"
#endif

#ifdef EDITOR_MODE
#include <foe/external/imgui.h>
#include <foe/imgui/state.hpp>
#include <foe/imgui/vk/renderer.hpp>
#include <foe/simulation/imgui/group_data.hpp>
#include <foe/simulation/imgui/registrar.hpp>

#include "imgui/developer_console.hpp"
#include "imgui/entity_list.hpp"
#include "imgui/frame_time_info.hpp"
#include "imgui/resource_list.hpp"
#include "imgui/save.hpp"
#include "imgui/termination.hpp"
#include "imgui/window.hpp"

#ifdef IMGUI_SHOW_DEMO
#include "imgui/demo.hpp"
#endif
#endif

struct foeSimulation;

struct Application {
#ifdef FOE_SKUNKWORKS_QT
    void setQtGuiApplication(QGuiApplication *pQtGuiApplication);

    struct ImportedQtWindowData {
        foeQtVulkanWindow *pQtWindow;
        std::atomic_bool *pNeedSwapchainBuild;
        std::atomic_bool *pActive;
    };

    void setQtWindows(std::vector<ImportedQtWindowData> const &startingWindows);
#endif

    int initialize(int argc, char **argv);
    void deinitialize();

    int mainloop();

    // Variables
    Settings settings;

    foeSplitThreadPool threadPool{FOE_NULL_HANDLE};
    foeSearchPaths searchPaths;

    // Groups/Entities
    foeSimulation *pSimulationSet{nullptr};

    // I/O
#ifdef FOE_SKUNKWORKS_GLFW
    std::vector<GLFW_WindowData *> glfw_windowData;
#endif
#ifdef FOE_SKUNKWORKS_SDL3
    std::vector<SDL3_WindowData *> sdl3_windowData;
#endif
#ifdef FOE_SKUNKWORKS_QT
    QGuiApplication *pQtGuiApplication = nullptr;
    std::mutex qt_windowDataSync;
    std::vector<Qt_WindowData *> qt_windowData;
#endif
    FrameTimer frameTime;

    // XR
    foeXrRuntime xrRuntime{FOE_NULL_HANDLE};
#ifdef FOE_XR_SUPPORT
    BringupAppXrData xrData;
#endif

    // Gfx
    foeGfxRuntime gfxRuntime{FOE_NULL_HANDLE};
    foeGfxSession gfxSession{FOE_NULL_HANDLE};
    foeGfxUploadContext gfxResUploadContext{FOE_NULL_HANDLE};
    foeGfxDelayedCaller gfxDelayedDestructor{FOE_NULL_HANDLE};
    VkFormat depthFormat{VK_FORMAT_D16_UNORM};
    foeGfxRenderViewPool gfxRenderViewPool{FOE_NULL_HANDLE};

    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

#ifdef EDITOR_MODE
    foeImGuiRenderer imguiRenderer;
    foeImGuiState imguiState;
    foeSimulationImGuiRegistrar imguiRegistrar;
    void *pImGuiRenderWindow = nullptr;

    foeImGuiDeveloperConsole devConsole;
    foeImGuiTermination fileTermination;
    foeImGuiFrameTimeInfo viewFrameTimeInfo{&frameTime};
    foeImGuiWindow windowInfo;
    foeImGuiSave uiSave;

#ifdef IMGUI_SHOW_DEMO
    foeImGuiDemo demo;
#endif

    // Per SimState UI
    std::unique_ptr<foeSimulationImGuiGroupData> pSimGroupDataUI;
    std::unique_ptr<foeImGuiEntityList> pEntityListUI;
    std::unique_ptr<foeImGuiResourceList> pResourceListUI;
#endif
};

#endif // APPLICATION_HPP