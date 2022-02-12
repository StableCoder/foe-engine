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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/delayed_destructor.hpp>
#include <foe/graphics/runtime.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/split_thread_pool.hpp>
#include <foe/xr/runtime.hpp>

#include "per_frame_data.hpp"
#include "settings.hpp"
#include "window.hpp"

#include <array>
#include <map>
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
#endif

struct foeSimulationState;

struct Application {
    auto initialize(int argc, char **argv) -> std::tuple<bool, int>;
    void deinitialize();

    std::error_code startXR(bool localPoll);
    std::error_code stopXR(bool localPoll);

    int mainloop();

    // Variables
    Settings settings;

    foeSplitThreadPool threadPool{FOE_NULL_HANDLE};

    // Groups/Entities
    foeId cameraID = FOE_INVALID_ID;

    foeSimulationState *pSimulationSet{nullptr};

    // I/O
    std::array<WindowData, 1> windowData;
    FrameTimer frameTime;

    // XR
    foeXrRuntime xrRuntime{FOE_NULL_HANDLE};
#ifdef FOE_XR_SUPPORT
    foeXrSession xrSession{};
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
    foeGfxDelayedDestructor gfxDelayedDestructor{FOE_NULL_HANDLE};
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

    // Per SimState UI
    std::unique_ptr<foeSimulationImGuiGroupData> pSimGroupDataUI;
    std::unique_ptr<foeImGuiEntityList> pEntityListUI;
    std::unique_ptr<foeImGuiResourceList> pResourceListUI;
#endif

    VkSwapchainKHR framebufferSwapchain{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> swapImageFramebuffers;
};

#endif // APPLICATION_HPP