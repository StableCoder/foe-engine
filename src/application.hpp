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

#include <foe/graphics/render_pass_pool.hpp>
#include <foe/graphics/runtime.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/swapchain.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/simulation/state.hpp>
#include <foe/thread_pool.hpp>
#include <foe/wsi.hpp>
#include <foe/xr/runtime.hpp>

#include "frame_timer.hpp"
#include "per_frame_data.hpp"
#include "settings.hpp"

#include <array>
#include <map>
#include <tuple>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/session.hpp>
#include <foe/xr/vulkan.hpp>

#include "xr_camera.hpp"
#include "xr_vk_camera_system.hpp"
#include "xr_vk_session_view.hpp"
#endif

#ifdef EDITOR_MODE
#include <foe/imgui/renderer.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include "imgui/frame_time_info.hpp"
#include "imgui/termination.hpp"
#endif

struct Application {
    auto initialize(int argc, char **argv) -> std::tuple<bool, int>;
    void deinitialize();

    int mainloop();

    // Variables
    Settings settings;

    foeThreadPool synchronousThreadPool;
    foeThreadPool asynchronousThreadPool;

    // Groups/Entities
    foeId cameraID = FOE_INVALID_ID;
    foeId renderTriangleID = FOE_INVALID_ID;
    foeId renderMeshID = FOE_INVALID_ID;

    std::unique_ptr<foeSimulationState, std::function<void(foeSimulationState *)>> pSimulationSet;

    FrameTimer frameTime;

    foeXrRuntime xrRuntime{FOE_NULL_HANDLE};

#ifdef FOE_XR_SUPPORT
    foeXrSession xrSession{};
    VkRenderPass xrRenderPass;
    std::vector<foeXrVkSessionView> xrViews;
    foeXrVkCameraSystem xrVkCameraSystem;
#endif

    foeGfxRuntime gfxRuntime{FOE_NULL_HANDLE};
    foeGfxSession gfxSession{FOE_NULL_HANDLE};
    foeGfxUploadContext resUploader{FOE_NULL_HANDLE};

    VkSurfaceKHR windowSurface{VK_NULL_HANDLE};
    foeSwapchain swapchain;

    uint32_t frameIndex = 0;
    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

    foeRenderPassPool renderPassPool;

    foeGfxVkPipelinePool pipelinePool;

#ifdef EDITOR_MODE
    foeImGuiRenderer imguiRenderer;
    foeImGuiState imguiState;

    foeImGuiTermination fileTermination;
    foeImGuiFrameTimeInfo viewFrameTimeInfo{&frameTime};
#endif

    std::vector<VkFramebuffer> swapImageFramebuffers;
    bool swapchainRebuilt = false;
};

#endif // APPLICATION_HPP