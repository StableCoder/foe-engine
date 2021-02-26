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

#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/graphics/render_pass_pool.hpp>
#include <foe/graphics/runtime.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/shader.hpp>
#include <foe/graphics/swapchain.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/resource/image_loader.hpp>
#include <foe/resource/image_pool.hpp>
#include <foe/resource/material_loader.hpp>
#include <foe/resource/material_pool.hpp>
#include <foe/resource/shader_loader.hpp>
#include <foe/resource/shader_pool.hpp>
#include <foe/resource/vertex_descriptor.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>
#include <foe/resource/vertex_descriptor_pool.hpp>
#include <foe/thread_pool.hpp>
#include <foe/wsi.hpp>
#include <foe/xr/runtime.hpp>

#include "camera.hpp"
#include "camera_descriptor_pool.hpp"
#include "frame_timer.hpp"
#include "per_frame_data.hpp"
#include "settings.hpp"

#include <array>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/session.hpp>
#include <foe/xr/vulkan.hpp>

#include "xr_camera.hpp"
#endif

#ifdef EDITOR_MODE
#include <foe/imgui/renderer.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include "imgui/frame_time_info.hpp"
#include "imgui/termination.hpp"
#endif

struct Application {
    int initialize(int argc, char **argv);
    void deinitialize();

    int mainloop();

    // Variables
    Settings settings;

    foeThreadPool synchronousThreadPool;
    foeThreadPool asynchronousThreadPool;
    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock{std::chrono::nanoseconds{0}};

    FrameTimer frameTime;

    foeXrRuntime xrRuntime{FOE_NULL_HANDLE};

#ifdef FOE_XR_SUPPORT
    struct foeXrSessionView {
        XrViewConfigurationView viewConfig;
        XrSwapchain swapchain;
        VkFormat format;
        std::vector<XrSwapchainImageVulkanKHR> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;
        foeXrCamera camera;
    };

    foeXrSession xrSession{};
    VkRenderPass xrRenderPass;
    std::vector<foeXrSessionView> xrViews;
#endif

    foeGfxRuntime gfxRuntime{FOE_NULL_HANDLE};
    foeGfxSession gfxSession{FOE_NULL_HANDLE};

    foeGfxUploadContext resUploader;

    VkSurfaceKHR windowSurface{VK_NULL_HANDLE};
    foeSwapchain swapchain;

    uint32_t frameIndex = 0;
    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

    foeRenderPassPool renderPassPool;

    foeGfxVkFragmentDescriptorPool fragmentDescriptorPool;
    foeGfxVkPipelinePool pipelinePool;

    // Resources
    foeShaderLoader shaderLoader;
    foeShaderPool shaderPool;
    foeVertexDescriptorLoader vertexDescriptorLoader;
    foeVertexDescriptorPool vertexDescriptorPool;
    foeImageLoader imageLoader;
    foeImagePool imagePool;
    foeMaterialLoader materialLoader;
    foeMaterialPool materialPool;

    Camera camera;
    CameraDescriptorPool cameraDescriptorPool;

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