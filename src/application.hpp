// Copyright (C) 2021-2022 George Cave.
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

#include "per_frame_data.hpp"
#include "settings.hpp"
#include "window.hpp"

#include <array>
#include <memory>

#ifdef FOE_XR_SUPPORT
#include "xr.hpp"
#endif

#ifdef EDITOR_MODE
#include <foe/imgui/state.hpp>
#include <foe/imgui/vk/renderer.hpp>
#include <foe/simulation/imgui/group_data.hpp>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/wsi/imgui/window.hpp>
#include <imgui.h>

#include "imgui/developer_console.hpp"
#include "imgui/entity_list.hpp"
#include "imgui/frame_time_info.hpp"
#include "imgui/resource_list.hpp"
#include "imgui/save.hpp"
#include "imgui/termination.hpp"

#ifdef IMGUI_SHOW_DEMO
#include "imgui/demo.hpp"
#endif
#endif

struct foeSimulation;

struct Application {
    auto initialize(int argc, char **argv) -> std::tuple<bool, int>;
    void deinitialize();

    int mainloop();

    // Variables
    Settings settings;

    foeSplitThreadPool threadPool{FOE_NULL_HANDLE};
    foeSearchPaths searchPaths;

    // Groups/Entities
    foeSimulation *pSimulationSet{nullptr};

    // I/O
    std::array<WindowData, 2> windowData;
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
    VkSampleCountFlags globalMSAA;
    foeGfxRenderViewPool gfxRenderViewPool{FOE_NULL_HANDLE};

    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

#ifdef EDITOR_MODE
    foeImGuiRenderer imguiRenderer;
    foeImGuiState imguiState;
    foeSimulationImGuiRegistrar imguiRegistrar;

    foeImGuiDeveloperConsole devConsole;
    foeImGuiTermination fileTermination;
    foeImGuiFrameTimeInfo viewFrameTimeInfo{&frameTime};
    foeWsiImGuiWindow windowInfo;
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