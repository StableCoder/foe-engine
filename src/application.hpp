// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/runtime.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/split_thread_pool.h>
#include <foe/xr/runtime.h>

#include "per_frame_data.hpp"
#include "settings.hpp"
#include "window.hpp"

#include <array>
#include <map>
#include <memory>
#include <tuple>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/session.hpp>
#include <foe/xr/openxr/vk/vulkan.hpp>

#include "xr_camera.hpp"
#include "xr_vk_camera_system.hpp"
#include "xr_vk_session_view.hpp"
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

    foeResultSet startXR(bool localPoll);
    foeResultSet stopXR(bool localPoll);

    int mainloop();

    // Variables
    Settings settings;

    foeSplitThreadPool threadPool{FOE_NULL_HANDLE};

    // Groups/Entities
    foeId cameraID = FOE_INVALID_ID;

    foeSimulation *pSimulationSet{nullptr};

    // I/O
    std::array<WindowData, 1> windowData;
    FrameTimer frameTime;

    // XR
    foeXrRuntime xrRuntime{FOE_NULL_HANDLE};
#ifdef FOE_XR_SUPPORT
    foeOpenXrSession xrSession{};
    VkRenderPass xrRenderPass;
    XrFrameState xrFrameState;
    std::vector<foeXrVkSessionView> xrViews;
    VkRenderPass xrOffscreenRenderPass;
    std::vector<foeGfxRenderTarget> xrOffscreenRenderTargets;
    foeXrVkCameraSystem xrVkCameraSystem;
#endif

    // Gfx
    foeGfxRuntime gfxRuntime{FOE_NULL_HANDLE};
    foeGfxSession gfxSession{FOE_NULL_HANDLE};
    foeGfxUploadContext gfxResUploadContext{FOE_NULL_HANDLE};
    foeGfxDelayedCaller gfxDelayedDestructor{FOE_NULL_HANDLE};
    VkFormat depthFormat{VK_FORMAT_D16_UNORM};
    VkSampleCountFlags globalMSAA;

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

    VkSwapchainKHR framebufferSwapchain{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> swapImageFramebuffers;
};

#endif // APPLICATION_HPP