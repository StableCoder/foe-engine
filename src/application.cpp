// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "application.hpp"

#include <GLFW/glfw3.h>
#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/ecs/name_map.h>
#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/vk/mesh.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/job/blit_image.hpp>
#include <foe/graphics/vk/render_graph/job/export_image.hpp>
#include <foe/graphics/vk/render_graph/job/import_image.hpp>
#include <foe/graphics/vk/render_graph/job/present_image.hpp>
#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/sample_count.h>
#include <foe/graphics/vk/session.h>
#include <foe/imex/exporters.h>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/system.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/quaternion_math.hpp>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>
#include <foe/wsi/keyboard.hpp>
#include <foe/wsi/mouse.hpp>
#include <foe/wsi/vulkan.h>

#include "foe/handle.h"
#include "graphics.hpp"
#include "log.hpp"
#include "logging.hpp"
#include "register_basic_functionality.h"
#include "simulation/armature_system.hpp"
#include "simulation/camera_pool.hpp"
#include "simulation/camera_system.hpp"
#include "simulation/position_descriptor_pool.hpp"
#include "simulation/vk_animation.hpp"

#include "render_graph/render_scene.hpp"

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/core.hpp>
#include <foe/xr/openxr/runtime.h>
#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include "xr.hpp"
#include "xr_result.h"
#endif

#ifdef EDITOR_MODE
#include <foe/imgui/vk/render_graph_job_imgui.hpp>

#include "imgui/register.hpp"
#endif

#ifdef WSI_LOADER
#include <foe/wsi/loader.h>
#endif

#include <thread>

#define ERRC_END_PROGRAM_TUPLE                                                                     \
    {                                                                                              \
        char buffer[FOE_MAX_RESULT_STRING_SIZE];                                                   \
        result.toString(result.value, buffer);                                                     \
        FOE_LOG(General, Fatal, "End called from {}:{} with error: {}", __FILE__, __LINE__,        \
                buffer);                                                                           \
        return std::make_tuple(false, result.value);                                               \
    }

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        char buffer[FOE_MAX_RESULT_STRING_SIZE];                                                   \
        result.toString(result.value, buffer);                                                     \
        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,         \
                buffer);                                                                           \
        return result.value;                                                                       \
    }

#define END_PROGRAM_TUPLE                                                                          \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{}", __FILE__, __LINE__);                      \
        return std::make_tuple(false, 1);                                                          \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{}", __FILE__, __LINE__);                      \
        return 1;                                                                                  \
    }

#include "state_import/import_state.hpp"

auto Application::initialize(int argc, char **argv) -> std::tuple<bool, int> {
    foeResultSet result;

    initializeLogging();
    result = registerBasicFunctionality();
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(General, Fatal, "Error registering basic functionality: {}", buffer)
        return std::make_tuple(false, result.value);
    }

    auto [continueRun, retVal] = loadSettings(argc, argv, settings, searchPaths);
    if (!continueRun) {
        return std::make_tuple(false, retVal);
    }

    result = foeCreateThreadPool(1, 1, &threadPool);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM_TUPLE

    result = foeStartThreadPool(threadPool);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM_TUPLE

    result = importState("persistent", &searchPaths, &pSimulationSet);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(General, Fatal, "Error importing '{}' state with error: {}", "persistent", buffer)

        return std::make_tuple(false, result.value);
    }

    // Special Entities
    foeEcsNameMapFindID(pSimulationSet->entityNameMap, "camera", &cameraID);

#ifdef EDITOR_MODE
    auto *pImGuiContext = ImGui::CreateContext();

    result = registerImGui(&imguiRegistrar);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM_TUPLE

    foeLogger::instance()->registerSink(&devConsole);

    imguiState.setImGuiContext(pImGuiContext);
    imguiRenderer.setImGuiContext(pImGuiContext);

    uiSave.registerUI(&imguiState);
    devConsole.registerUI(&imguiState);
    fileTermination.registerUI(&imguiState);
    viewFrameTimeInfo.registerUI(&imguiState);
    windowInfo.registerUI(&imguiState);

#ifdef IMGUI_SHOW_DEMO
    demo.registerUI(&imguiState);
#endif

    uiSave.setSimulationState(pSimulationSet);

    // Per SimState UI
    pSimGroupDataUI.reset(new foeSimulationImGuiGroupData{pSimulationSet});
    pSimGroupDataUI->registerUI(&imguiState);

    pEntityListUI.reset(new foeImGuiEntityList{pSimulationSet, &imguiRegistrar});
    pEntityListUI->registerUI(&imguiState);

    pResourceListUI.reset(new foeImGuiResourceList{pSimulationSet, &imguiRegistrar});
    pResourceListUI->registerUI(&imguiState);
#endif

#ifdef WSI_LOADER
    std::string wsiImplementation = DEFAULT_WSI_IMPLEMENTATION;
    if (!settings.window.implementation.empty())
        wsiImplementation = settings.window.implementation;

    if (!foeWsiLoadImplementation(wsiImplementation.data())) {
        END_PROGRAM_TUPLE
    }
#endif

    {
        for (auto &it : windowData) {
            result = foeWsiCreateWindow(settings.window.width, settings.window.height, "FoE Engine",
                                        true, &it.window);
            if (result.value != FOE_SUCCESS)
                ERRC_END_PROGRAM_TUPLE

#ifdef EDITOR_MODE
            windowInfo.addWindow(it.window);
#endif
        }

#ifdef FOE_XR_SUPPORT
        if (settings.xr.enableXr || settings.xr.forceXr) {
            result = createXrRuntime(settings.xr.debugLogging, &xrRuntime);
            if (result.value != FOE_SUCCESS && settings.xr.forceXr) {
                ERRC_END_PROGRAM_TUPLE
            }
        }
#endif

        result =
            createGfxRuntime(xrRuntime, settings.window.enableWSI, settings.graphics.validation,
                             settings.graphics.debugLogging, &gfxRuntime);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }

        for (auto &it : windowData) {
            result = foeWsiWindowGetVkSurface(it.window, foeGfxVkGetRuntimeInstance(gfxRuntime),
                                              &it.surface);
            if (result.value != FOE_SUCCESS)
                ERRC_END_PROGRAM_TUPLE
        }

        std::vector<VkSurfaceKHR> surfaces;
        for (auto const &it : windowData) {
            surfaces.push_back(it.surface);
        }

        result =
            createGfxSession(gfxRuntime, xrRuntime, settings.window.enableWSI, std::move(surfaces),
                             settings.graphics.gpu, settings.xr.forceXr, &gfxSession);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }

        // Make sure the MSAA setting is valid
        globalMSAA = foeGfxVkGetMaxSupportedMSAA(gfxSession);
        int maxSupportedCount = foeGfxVkGetSampleCount(globalMSAA);

        if (settings.graphics.msaa > maxSupportedCount) {
            settings.graphics.msaa = maxSupportedCount;
        } else if (settings.graphics.msaa < 1) {
            settings.graphics.msaa = 1;
        }
        do {
            globalMSAA = foeGfxVkGetSampleCountFlags(settings.graphics.msaa);
            if (globalMSAA == 0)
                --settings.graphics.msaa;
        } while (globalMSAA == 0);
    }

    result = foeGfxCreateUploadContext(gfxSession, &gfxResUploadContext);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM_TUPLE
    }

    result = foeGfxCreateDelayedCaller(gfxSession, FOE_GRAPHICS_MAX_BUFFERED_FRAMES,
                                       &gfxDelayedDestructor);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM_TUPLE
    }

#ifdef EDITOR_MODE
    imguiRenderer.resize(settings.window.width, settings.window.height);
    float xScale, yScale;
    foeWsiWindowGetContentScale(windowData[0].window, &xScale, &yScale);
    imguiRenderer.rescale(xScale, yScale);
#endif

    // Create per-frame data
    for (auto &it : frameData) {
        result = it.create(foeGfxVkGetDevice(gfxSession));
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }
    }

    { // Initialize simulation
        foeSimulationInitInfo simInitInfo{
            .externalFileSearchFn =
                std::bind(&foeGroupData::findExternalFile, &pSimulationSet->groupData,
                          std::placeholders::_1, std::placeholders::_2),
        };
        foeInitializeSimulation(pSimulationSet, &simInitInfo);

        foeInitializeSimulationGraphics(pSimulationSet, gfxSession);
    }

    { // Load all available resources
        auto asyncTaskFunc = [](void *pScheduleContext, PFN_foeTask task, void *pTaskContext) {
            auto threadPool = reinterpret_cast<foeSplitThreadPool>(pScheduleContext);

            foeScheduleAsyncTask(threadPool, task, pTaskContext);
        };

        foeResourcePoolAddAsyncTaskCallback(pSimulationSet->resourcePool, asyncTaskFunc,
                                            (void *)threadPool);
    }

#ifdef FOE_XR_SUPPORT
    if (settings.xr.enableXr || settings.xr.forceXr) {
        startXR(true);

        // If the user specified to force an XR session and couldn't find/create the session, fail
        // out
        if (settings.xr.forceXr && xrSession == FOE_NULL_HANDLE) {
            FOE_LOG(General, Fatal, "XR support enabled but no HMD session was detected/started.")
            return std::make_tuple(false, 1);
        }
    }
#endif

    return std::make_tuple(true, FOE_SUCCESS);
}

void Application::deinitialize() {
    foeResultSet result;

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxWaitIdle(gfxSession);

    // Deinit simulation
    if (pSimulationSet) {
        foeDeinitializeSimulationGraphics(pSimulationSet);
        foeDeinitializeSimulation(pSimulationSet);
    }

#ifdef FOE_XR_SUPPORT
    stopXR(true);

    if (xrRuntime != FOE_NULL_HANDLE)
        result = foeXrDestroyRuntime(xrRuntime);
#endif

    // Cleanup per-frame data
    if (gfxSession != FOE_NULL_HANDLE) {
        for (auto &it : frameData)
            it.destroy(foeGfxVkGetDevice(gfxSession));
        for (auto &it : swapImageFramebuffers)
            vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);

#ifdef EDITOR_MODE
        imguiRenderer.deinitialize(gfxSession);
#endif
    }

    // Destroy window data
    for (auto &it : windowData) {
#ifdef EDITOR_MODE
        windowInfo.removeWindow(it.window);
#endif
        if (it.gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
            foeGfxDestroyRenderTarget(it.gfxOffscreenRenderTarget);
        it.gfxOffscreenRenderTarget = FOE_NULL_HANDLE;

        if (gfxSession != FOE_NULL_HANDLE)
            it.swapchain.destroy(foeGfxVkGetDevice(gfxSession));

        if (it.surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime), it.surface, nullptr);
        it.surface = VK_NULL_HANDLE;

        if (it.window != FOE_NULL_HANDLE)
            foeWsiDestroyWindow(it.window);
        it.window = FOE_NULL_HANDLE;
    }

    // Cleanup graphics
    if (gfxDelayedDestructor != FOE_NULL_HANDLE)
        foeGfxDestroyDelayedCaller(gfxDelayedDestructor);
    gfxDelayedDestructor = FOE_NULL_HANDLE;

    if (gfxResUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(gfxResUploadContext);
    gfxResUploadContext = FOE_NULL_HANDLE;

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxDestroySession(gfxSession);
    gfxSession = FOE_NULL_HANDLE;

    if (gfxRuntime != FOE_NULL_HANDLE)
        foeGfxDestroyRuntime(gfxRuntime);
    gfxRuntime = FOE_NULL_HANDLE;

    // Cleanup threadpool
    if (threadPool)
        foeDestroyThreadPool(threadPool);
    threadPool = FOE_NULL_HANDLE;

#ifdef EDITOR_MODE
    deregisterImGui(&imguiRegistrar);
#endif

    if (pSimulationSet != nullptr)
        foeDestroySimulation(pSimulationSet);
    pSimulationSet = nullptr;

    // Deregister functionality
    deregisterBasicFunctionality();

#ifdef EDITOR_MODE
    foeLogger::instance()->deregisterSink(&devConsole);
#endif

    // Output configuration settings to a YAML configuration file
    // saveSettings(settings);
}

namespace {

void processUserInput(double timeElapsedInSeconds,
                      foeWsiKeyboard const *pKeyboard,
                      foeWsiMouse const *pMouse,
                      foePosition3d *pCamera) {
    constexpr float movementMultiplier = 10.f;
    constexpr float rorationMultiplier = 40.f;
    float multiplier = timeElapsedInSeconds * 3.f; // 3 units per second

    if (pMouse->inWindow) {
        if (pKeyboard->keyDown(GLFW_KEY_Z)) { // Up
            pCamera->position += upVec(pCamera->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_X)) { // Down
            pCamera->position -= upVec(pCamera->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keyDown(GLFW_KEY_W)) { // Forward
            pCamera->position += forwardVec(pCamera->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_S)) { // Back
            pCamera->position -= forwardVec(pCamera->orientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keyDown(GLFW_KEY_A)) { // Left
            pCamera->position += leftVec(pCamera->orientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_D)) { // Right
            pCamera->position -= leftVec(pCamera->orientation) * movementMultiplier * multiplier;
        }

        if (pMouse->buttonDown(GLFW_MOUSE_BUTTON_1)) {
            pCamera->orientation = changeYaw(
                pCamera->orientation, -glm::radians(pMouse->oldPosition.x - pMouse->position.x));
            pCamera->orientation = changePitch(
                pCamera->orientation, glm::radians(pMouse->oldPosition.y - pMouse->position.y));

            if (pKeyboard->keyDown(GLFW_KEY_Q)) { // Roll Left
                pCamera->orientation =
                    changeRoll(pCamera->orientation, glm::radians(rorationMultiplier * multiplier));
            }
            if (pKeyboard->keyDown(GLFW_KEY_E)) { // Roll Right
                pCamera->orientation = changeRoll(pCamera->orientation,
                                                  -glm::radians(rorationMultiplier * multiplier));
            }
        }
    }
}

} // namespace

foeResultSet Application::startXR(bool localPoll) {
    foeResultSet result{.value = FOE_SUCCESS, .toString = NULL};

    if (xrRuntime == FOE_NULL_HANDLE) {
        FOE_LOG(General, Error, "Tried to start an XR session, but no XR runtime has been started");
    }
#ifdef FOE_XR_SUPPORT
    else {
        result = createXrSession(xrRuntime, gfxSession, &xrSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }

        // OpenXR Session Begin

        // Wait for the session to be ready
        while (foeOpenXrGetSessionState(xrSession) != XR_SESSION_STATE_READY) {
            if (localPoll) {
                result = foeXrProcessEvents(xrRuntime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);

                    goto START_XR_FAILED;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        // Session Views
        uint32_t viewConfigViewCount;
        result = xr_to_foeResult(xrEnumerateViewConfigurationViews(
            foeOpenXrGetInstance(xrRuntime), foeOpenXrGetSystemId(xrSession),
            foeOpenXrGetViewConfigurationType(xrSession), 0, &viewConfigViewCount, nullptr));
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }

        std::vector<XrViewConfigurationView> viewConfigs;
        viewConfigs.resize(viewConfigViewCount);

        result = xr_to_foeResult(xrEnumerateViewConfigurationViews(
            foeOpenXrGetInstance(xrRuntime), foeOpenXrGetSystemId(xrSession),
            foeOpenXrGetViewConfigurationType(xrSession), viewConfigs.size(), &viewConfigViewCount,
            viewConfigs.data()));
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }
        xrViews.clear();
        for (auto const &it : viewConfigs) {
            xrViews.emplace_back(foeXrVkSessionView{.viewConfig = it});
        }

        // OpenXR Swapchains
        std::vector<int64_t> swapchainFormats;
        result =
            foeOpenXrEnumerateSwapchainFormats(foeOpenXrGetSession(xrSession), swapchainFormats);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }
        for (auto &it : xrViews) {
            it.format = static_cast<VkFormat>(swapchainFormats[0]);
        }

        auto *renderPassPool = foeGfxVkGetRenderPassPool(gfxSession);
        xrRenderPass = renderPassPool->renderPass({VkAttachmentDescription{
            .format = static_cast<VkFormat>(swapchainFormats[0]),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        }});

        xrOffscreenRenderPass = renderPassPool->renderPass(
            {VkAttachmentDescription{
                 .format = static_cast<VkFormat>(swapchainFormats[0]),
                 .samples = static_cast<VkSampleCountFlagBits>(globalMSAA),
                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                 .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
             },
             VkAttachmentDescription{
                 .format = depthFormat,
                 .samples = static_cast<VkSampleCountFlagBits>(globalMSAA),
                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                 .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             }});

        for (auto &view : xrViews) {
            // Offscreen Render Targets
            std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
                foeGfxVkRenderTargetSpec{
                    .format = view.format,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .count = 3,
                },
                foeGfxVkRenderTargetSpec{
                    .format = depthFormat,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .count = 3,
                },
            };

            foeGfxRenderTarget newRenderTarget{FOE_NULL_HANDLE};
            result =
                foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedDestructor, offscreenSpecs.data(),
                                           offscreenSpecs.size(), globalMSAA, &newRenderTarget);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        buffer);

                goto START_XR_FAILED;
            }

            foeGfxUpdateRenderTargetExtent(newRenderTarget,
                                           view.viewConfig.recommendedImageRectWidth,
                                           view.viewConfig.recommendedImageRectHeight);

            result =
                foeGfxAcquireNextRenderTarget(newRenderTarget, FOE_GRAPHICS_MAX_BUFFERED_FRAMES);

            xrOffscreenRenderTargets.emplace_back(newRenderTarget);

            // Swapchain
            XrSwapchainCreateInfo swapchainCI{
                .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
                .createFlags = 0,
                .usageFlags =
                    XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT,
                .format = view.format,
                .sampleCount = 1,
                .width = view.viewConfig.recommendedImageRectWidth,
                .height = view.viewConfig.recommendedImageRectHeight,
                .faceCount = 1,
                .arraySize = 1,
                .mipCount = 1,
            };

            result = xr_to_foeResult(
                xrCreateSwapchain(foeOpenXrGetSession(xrSession), &swapchainCI, &view.swapchain));
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        buffer);

                goto START_XR_FAILED;
            }

            // Images
            result = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, view.images);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        buffer);

                goto START_XR_FAILED;
            }

            // Image Views
            VkImageViewCreateInfo viewCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = view.format,
                .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                               VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
                .subresourceRange =
                    VkImageSubresourceRange{
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
            };

            // VkImageView
            view.imageViews.clear();
            for (auto it : view.images) {
                viewCI.image = it.image;
                VkImageView vkView{VK_NULL_HANDLE};
                result = vk_to_foeResult(
                    vkCreateImageView(foeGfxVkGetDevice(gfxSession), &viewCI, nullptr, &vkView));
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);

                    goto START_XR_FAILED;
                }

                view.imageViews.emplace_back(vkView);
            }

            // VkFramebuffer
            for (auto it : view.imageViews) {
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = xrRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = &it,
                    .width = view.viewConfig.recommendedImageRectWidth,
                    .height = view.viewConfig.recommendedImageRectHeight,
                    .layers = 1,
                };

                VkFramebuffer newFramebuffer;
                result = vk_to_foeResult(vkCreateFramebuffer(
                    foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr, &newFramebuffer));
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);

                    goto START_XR_FAILED;
                }

                view.framebuffers.emplace_back(newFramebuffer);
            }
        }

        result = xrVkCameraSystem.initialize(gfxSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }

        result = foeOpenXrBeginSession(xrSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    buffer);

            goto START_XR_FAILED;
        }

        FOE_LOG(General, Info, "Started new XR session {}",
                static_cast<void *>(foeOpenXrGetSession(xrSession)));
    }

START_XR_FAILED:
    if (result.value != FOE_SUCCESS) {
        stopXR(localPoll);
    }
#endif // FOE_XR_SUPPORT

    return result;
}

foeResultSet Application::stopXR(bool localPoll) {
    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};

#ifdef FOE_XR_SUPPORT
    if (xrSession != FOE_NULL_HANDLE) {
        foeOpenXrRequestExitSession(xrSession);

        while (foeOpenXrGetSessionState(xrSession) != XR_SESSION_STATE_STOPPING) {
            if (localPoll) {
                result = foeXrProcessEvents(xrRuntime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);
                }
                return result;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        foeOpenXrEndSession(xrSession);

        while (foeOpenXrGetSessionState(xrSession) != XR_SESSION_STATE_IDLE) {
            if (localPoll) {
                result = foeXrProcessEvents(xrRuntime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);
                    return result;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        for (auto &renderTarget : xrOffscreenRenderTargets) {
            if (renderTarget != FOE_NULL_HANDLE)
                foeGfxDestroyRenderTarget(renderTarget);
        }
        xrOffscreenRenderTargets.clear();

        for (auto &view : xrViews) {
            for (auto it : view.imageViews) {
                vkDestroyImageView(foeGfxVkGetDevice(gfxSession), it, nullptr);
            }
            if (view.swapchain != XR_NULL_HANDLE) {
                xrDestroySwapchain(view.swapchain);
            }
        }

        while (foeOpenXrGetSessionState(xrSession) != XR_SESSION_STATE_EXITING) {
            if (localPoll) {
                result = foeXrProcessEvents(xrRuntime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, buffer);
                    return result;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        foeXrDestroySession(xrSession);
    }

#endif // FOE_XR_SUPPORT

    return result;
}

int Application::mainloop() {
    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock{std::chrono::nanoseconds{0}};
    foeResultSet result;

    uint32_t lastFrameIndex = UINT32_MAX;
    uint32_t frameIndex = UINT32_MAX;

    foeWsiWindowShow(windowData[0].window);
    programClock.update();
    simulationClock.externalTime(programClock.currentTime<std::chrono::nanoseconds>());

    FOE_LOG(General, Info, "Entering main loop")
    while (!foeWsiWindowGetShouldClose(windowData[0].window)
#ifdef EDITOR_MODE
           && !fileTermination.terminationRequested()
#endif
    ) {
        // Timing
        programClock.update();
        simulationClock.update(programClock.currentTime<std::chrono::nanoseconds>());
        double timeElapsedInSec = simulationClock.elapsed().count() * 0.000000001f;

        // Timed test items
        static auto nextFireTime =
            programClock.currentTime<std::chrono::seconds>() + std::chrono::seconds(1);
        if (programClock.currentTime<std::chrono::seconds>() > nextFireTime) {
            nextFireTime =
                programClock.currentTime<std::chrono::seconds>() + std::chrono::seconds(6);

            if constexpr (false) {
                // Import the desired content
                foeSimulation *tempSimulation = nullptr;
                result = importState("theDataA", &searchPaths, &tempSimulation);
                if (result.value != FOE_SUCCESS)
                    std::abort();

                for (auto &it : tempSimulation->componentPools) {
                    if (it.pMaintenanceFn) {
                        it.pMaintenanceFn(it.pComponentPool);
                    }
                }

                // Export the content
                uint32_t numExporters;
                foeImexGetExporters(&numExporters, nullptr);
                std::unique_ptr<foeExporter[]> exporters(new foeExporter[numExporters]);
                foeImexGetExporters(&numExporters, exporters.get());

                result = exporters[1].pExportFn("test-save", tempSimulation);
                if (result.value != FOE_SUCCESS)
                    std::abort();

                foeDestroySimulation(tempSimulation);
            }

#ifdef FOE_XR_SUPPORT
            if constexpr (false) {
                if (xrSession == FOE_NULL_HANDLE) {
                    foeScheduleAsyncTask(
                        threadPool,
                        [](void *pApplication) { ((Application *)pApplication)->startXR(false); },
                        this);
                } else {
                    foeScheduleAsyncTask(
                        threadPool,
                        [](void *pApplication) { ((Application *)pApplication)->stopXR(false); },
                        this);
                }
            }
#endif // FOE_XR_SUPPORT
        }

        // Component Pool Maintenance
        for (auto &it : pSimulationSet->componentPools) {
            if (it.pMaintenanceFn) {
                it.pMaintenanceFn(it.pComponentPool);
            }
        }

        // Resource Loader Maintenance
        for (auto &it : pSimulationSet->resourceLoaders) {
            if (it.pMaintenanceFn) {
                it.pMaintenanceFn(it.pLoader);
            }
        }

        // Process systems
        ((foeArmatureSystem *)foeSimulationGetSystem(pSimulationSet,
                                                     FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM))
            ->process(timeElapsedInSec);
        ((foePhysicsSystem *)foeSimulationGetSystem(pSimulationSet,
                                                    FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM))
            ->process(timeElapsedInSec);

        // Process Window Events
        for (auto &it : windowData)
            foeWsiWindowProcessing(it.window);
        foeWsiGlobalProcessing();

#ifdef FOE_XR_SUPPORT
        // Process XR Events
        if (xrRuntime != FOE_NULL_HANDLE)
            foeXrProcessEvents(xrRuntime);
#endif

#ifdef EDITOR_MODE
        // User input processing
        imguiRenderer.keyboardInput(foeWsiGetKeyboard(windowData[0].window));
        imguiRenderer.mouseInput(foeWsiGetMouse(windowData[0].window));
        if (!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse())
#endif
        {
            auto pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                pSimulationSet, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

            auto *pCameraPosition = (pPosition3dPool->begin<1>() + pPosition3dPool->find(cameraID));
            processUserInput(timeElapsedInSec, foeWsiGetKeyboard(windowData[0].window),
                             foeWsiGetMouse(windowData[0].window), pCameraPosition->get());
        }

        // Check if windows were resized, and if so request associated swapchains to be rebuilt
        for (auto &it : windowData) {
            if (foeWsiWindowResized(it.window)) {
                it.swapchain.requestRebuild();

#ifdef EDITOR_MODE
                if (it.window == windowData[0].window) {
                    int width, height;
                    foeWsiWindowGetSize(windowData[0].window, &width, &height);
                    imguiRenderer.resize(width, height);
                }
#endif
            }
        }

        // Determine if the next frame is available to start rendering to, if we don't have one
        if (frameIndex == UINT32_MAX) {
            uint32_t nextFrameIndex = (lastFrameIndex + 1) % frameData.size();
            if (vkWaitForFences(foeGfxVkGetDevice(gfxSession), 1,
                                &frameData[nextFrameIndex].frameComplete, VK_TRUE,
                                0) == VK_SUCCESS) {
                frameIndex = nextFrameIndex;

                // Resource Loader Gfx Maintenance
                for (auto &it : pSimulationSet->resourceLoaders) {
                    if (it.pGfxMaintenanceFn) {
                        it.pGfxMaintenanceFn(it.pLoader);
                    }
                }

                // Reset frame data
                vkResetFences(foeGfxVkGetDevice(gfxSession), 1,
                              &frameData[frameIndex].frameComplete);
                vkResetCommandPool(foeGfxVkGetDevice(gfxSession), frameData[frameIndex].commandPool,
                                   0);

                // Advance and destroy items related to this frame
                foeGfxRunDelayedCalls(gfxDelayedDestructor);
            }
        }

        // If we have a frame we can render to, proceed to check for ready-to-render data
        if (frameIndex != UINT32_MAX) {
#ifdef FOE_XR_SUPPORT
            // Lock rendering to OpenXR framerate, which overrides regular rendering
            bool xrAcquiredFrame = false;
            if (xrSession != FOE_NULL_HANDLE && foeOpenXrGetSessionActive(xrSession)) {
                xrAcquiredFrame = true;

                XrFrameWaitInfo frameWaitInfo{.type = XR_TYPE_FRAME_WAIT_INFO};
                xrFrameState = XrFrameState{.type = XR_TYPE_FRAME_STATE};
                result = xr_to_foeResult(
                    xrWaitFrame(foeOpenXrGetSession(xrSession), &frameWaitInfo, &xrFrameState));
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }
            } else
#endif
            {
                // Artificially slow down for ImGui
                std::this_thread::sleep_for(std::chrono::milliseconds(14));
            }

            // Swapchain updates if necessary
            for (auto &it : windowData) {
                // If no window here, skip
                if (it.window == FOE_NULL_HANDLE)
                    continue;

                performWindowMaintenance(&it, gfxSession, gfxDelayedDestructor, globalMSAA,
                                         depthFormat);
            }

            { // All Cameras are currently ties to the primary window X/Y viewport size
                auto pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
                    pSimulationSet, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);

                int width, height;
                foeWsiWindowGetSize(windowData[0].window, &width, &height);
                for (auto *pCameraData = pCameraPool->begin<1>();
                     pCameraData != pCameraPool->end<1>(); ++pCameraData) {
                    pCameraData->get()->viewX = width;
                    pCameraData->get()->viewY = height;
                }
            }

            // Acquire Target Presentation Images
            std::vector<WindowData *> windowRenderList;
            windowRenderList.reserve(windowData.size());

            for (auto &it : windowData) {
                result =
                    vk_to_foeResult(it.swapchain.acquireNextImage(foeGfxVkGetDevice(gfxSession)));
                if (result.value == VK_TIMEOUT || result.value == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (result.value == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchains
                    it.swapchain.needRebuild();
                } else if (result.value == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it.swapchain.needRebuild();
                    windowRenderList.emplace_back(&it);
                } else if (result.value) {
                    // Catastrophic error
                    ERRC_END_PROGRAM
                } else {
                    // No issues, add it to be rendered
                    windowRenderList.emplace_back(&it);
                }
            }
            if (windowRenderList.empty()) {
                goto SKIP_FRAME_RENDER;
            }

            frameTime.newFrame();

            // Run Systems that generate graphics data
            ((foeCameraSystem *)foeSimulationGetSystem(pSimulationSet,
                                                       FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM))
                ->processCameras(frameIndex);

            ((PositionDescriptorPool *)foeSimulationGetSystem(
                 pSimulationSet, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL))
                ->generatePositionDescriptors(frameIndex);

            ((VkAnimationPool *)foeSimulationGetSystem(
                 pSimulationSet, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL))
                ->uploadBoneOffsets(frameIndex);

#ifdef FOE_XR_SUPPORT
            // OpenXR Render Section
            if (xrAcquiredFrame) {
                XrFrameBeginInfo frameBeginInfo{.type = XR_TYPE_FRAME_BEGIN_INFO};
                result =
                    xr_to_foeResult(xrBeginFrame(foeOpenXrGetSession(xrSession), &frameBeginInfo));
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                std::vector<XrCompositionLayerBaseHeader *> layers;
                std::vector<XrCompositionLayerProjectionView> projectionViews{
                    xrViews.size(), XrCompositionLayerProjectionView{
                                        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};
                XrCompositionLayerProjection layerProj;

                if (xrFrameState.shouldRender) {
                    XrViewLocateInfo viewLocateInfo{
                        .type = XR_TYPE_VIEW_LOCATE_INFO,
                        .viewConfigurationType = foeOpenXrGetViewConfigurationType(xrSession),
                        .displayTime = xrFrameState.predictedDisplayTime,
                        .space = foeOpenXrGetSpace(xrSession),
                    };
                    XrViewState viewState{.type = XR_TYPE_VIEW_STATE};
                    std::vector<XrView> views{xrViews.size(), XrView{.type = XR_TYPE_VIEW}};
                    uint32_t viewCountOutput;
                    result = xr_to_foeResult(
                        xrLocateViews(foeOpenXrGetSession(xrSession), &viewLocateInfo, &viewState,
                                      views.size(), &viewCountOutput, views.data()));
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }

                    for (size_t i = 0; i < views.size(); ++i) {
                        projectionViews[i].pose = views[i].pose;
                        projectionViews[i].fov = views[i].fov;

                        xrViews[i].camera.nearZ = 1;
                        xrViews[i].camera.farZ = 100;

                        xrViews[i].camera.fov = views[i].fov;
                        xrViews[i].camera.pose = views[i].pose;

                        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                            pSimulationSet, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

                        auto *pCameraPosition =
                            (pPosition3dPool->begin<1>() + pPosition3dPool->find(cameraID));
                        xrViews[i].camera.pPosition3D = pCameraPosition->get();
                    }

                    xrVkCameraSystem.processCameras(frameIndex, xrViews);

                    // Render Code
                    for (size_t i = 0; i < xrViews.size(); ++i) {
                        auto &it = xrViews[i];
                        auto &renderTarget = xrOffscreenRenderTargets[i];

                        result = foeGfxAcquireNextRenderTarget(renderTarget,
                                                               FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
                        if (result.value != FOE_SUCCESS)
                            ERRC_END_PROGRAM

                        XrSwapchainImageAcquireInfo acquireInfo{
                            .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};

                        uint32_t newIndex;
                        result = xr_to_foeResult(
                            xrAcquireSwapchainImage(it.swapchain, &acquireInfo, &newIndex));
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        projectionViews[i].subImage = XrSwapchainSubImage{
                            .swapchain = it.swapchain,
                            .imageRect =
                                XrRect2Di{
                                    .extent =
                                        {
                                            .width = static_cast<int32_t>(
                                                it.viewConfig.recommendedImageRectWidth),
                                            .height = static_cast<int32_t>(
                                                it.viewConfig.recommendedImageRectHeight),
                                        },
                                },
                        };

                        foeGfxVkRenderGraph renderGraph;
                        result = foeGfxVkCreateRenderGraph(&renderGraph);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphResource renderTargetColourImageResource;
                        foeGfxVkRenderGraphResource renderTargetDepthImageResource;

                        result = foeGfxVkImportImageRenderJob(
                            renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                            foeGfxVkGetRenderTargetImage(renderTarget, 0),
                            foeGfxVkGetRenderTargetImageView(renderTarget, 0), it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        result = foeGfxVkImportImageRenderJob(
                            renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                            "renderTargetDepthImage", foeGfxVkGetRenderTargetImage(renderTarget, 1),
                            foeGfxVkGetRenderTargetImageView(renderTarget, 1), depthFormat,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetDepthImageResource);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphResource xrSwapchainImageResource;
                        result = foeOpenXrVkImportSwapchainImageRenderJob(
                            renderGraph, "importXrViewSwapchainImage", VK_NULL_HANDLE,
                            "importXrViewSwapchainImage", it.swapchain, it.images[newIndex].image,
                            it.imageViews[newIndex], it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &xrSwapchainImageResource);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        RenderSceneOutputResources output;
                        result = renderSceneJob(
                            renderGraph, "render3dScene", VK_NULL_HANDLE,
                            renderTargetColourImageResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            renderTargetDepthImageResource,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, globalMSAA, pSimulationSet,
                            it.camera.descriptor, output);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        renderTargetColourImageResource = output.colourRenderTarget;
                        renderTargetDepthImageResource = output.depthRenderTarget;

                        if (foeGfxVkGetRenderTargetSamples(renderTarget) != VK_SAMPLE_COUNT_1_BIT) {
                            // Resolve
                            ResolveJobUsedResources resources;
                            result = foeGfxVkResolveImageRenderJob(
                                renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &resources);
                            if (result.value != FOE_SUCCESS) {
                                ERRC_END_PROGRAM
                            }

                            renderTargetColourImageResource = resources.srcImage;
                            xrSwapchainImageResource = resources.dstImage;
                        } else {
                            // Copy
                            BlitJobUsedResources resources;
                            result = foeGfxVkBlitImageRenderJob(
                                renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &resources);
                            if (result.value != FOE_SUCCESS) {
                                ERRC_END_PROGRAM
                            }

                            renderTargetColourImageResource = resources.srcImage;
                            xrSwapchainImageResource = resources.dstImage;
                        }

                        result = foeGfxVkExportImageRenderJob(
                            renderGraph, "exportPresentationImage", VK_NULL_HANDLE,
                            xrSwapchainImageResource, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {});
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        result = foeGfxVkExecuteRenderGraph(renderGraph, gfxSession,
                                                            gfxDelayedDestructor);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkExecuteRenderGraphCpuJobs(renderGraph);

                        foeGfxVkDestroyRenderGraph(renderGraph);
                    }

                    // Assemble composition layers
                    layerProj = XrCompositionLayerProjection{
                        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
                        .layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT,
                        .space = foeOpenXrGetSpace(xrSession),
                        .viewCount = static_cast<uint32_t>(projectionViews.size()),
                        .views = projectionViews.data(),
                    };
                    layers.emplace_back(
                        reinterpret_cast<XrCompositionLayerBaseHeader *>(&layerProj));
                }

                XrFrameEndInfo endFrameInfo{
                    .type = XR_TYPE_FRAME_END_INFO,
                    .displayTime = xrFrameState.predictedDisplayTime,
                    .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
                    .layerCount = static_cast<uint32_t>(layers.size()),
                    .layers = layers.data(),
                };
                result = xr_to_foeResult(xrEndFrame(foeOpenXrGetSession(xrSession), &endFrameInfo));
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }
            }
#endif

            auto &window = windowRenderList[0];

            result = foeGfxAcquireNextRenderTarget(window->gfxOffscreenRenderTarget,
                                                   FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            foeGfxVkRenderGraph renderGraph;
            result = foeGfxVkCreateRenderGraph(&renderGraph);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            foeGfxVkRenderGraphResource renderTargetColourImageResource;
            foeGfxVkRenderGraphResource renderTargetDepthImageResource;

            result = foeGfxVkImportImageRenderJob(
                renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 0),
                foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 0),
                window->swapchain.surfaceFormat().format, window->swapchain.extent(),
                VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            result = foeGfxVkImportImageRenderJob(
                renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                "renderTargetDepthImage",
                foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 1),
                foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 1), depthFormat,
                window->swapchain.extent(), VK_IMAGE_LAYOUT_UNDEFINED, true, {},
                &renderTargetDepthImageResource);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            auto *pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
                pSimulationSet, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);

            auto *pCamera = (pCameraPool->begin<1>() + pCameraPool->find(cameraID));

            RenderSceneOutputResources output;
            result = renderSceneJob(
                renderGraph, "render3dScene", VK_NULL_HANDLE, renderTargetColourImageResource,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderTargetDepthImageResource,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, globalMSAA, pSimulationSet,
                (*pCamera)->descriptor, output);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            renderTargetColourImageResource = output.colourRenderTarget;
            renderTargetDepthImageResource = output.depthRenderTarget;

            foeGfxVkRenderGraphResource presentImageResource;

            result = foeGfxVkImportSwapchainImageRenderJob(
                renderGraph, "importPresentationImage", VK_NULL_HANDLE, "presentImage",
                window->swapchain, window->swapchain.acquiredIndex(),
                window->swapchain.image(window->swapchain.acquiredIndex()),
                window->swapchain.imageView(window->swapchain.acquiredIndex()),
                window->swapchain.surfaceFormat().format, window->swapchain.extent(),
                VK_IMAGE_LAYOUT_UNDEFINED, window->swapchain.imageReadySemaphore(),
                &presentImageResource);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            if (foeGfxVkGetRenderTargetSamples(window->gfxOffscreenRenderTarget) !=
                VK_SAMPLE_COUNT_1_BIT) {
                // Resolve
                ResolveJobUsedResources resources;
                result = foeGfxVkResolveImageRenderJob(
                    renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                    renderTargetColourImageResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    presentImageResource, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resources);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                renderTargetColourImageResource = resources.srcImage;
                presentImageResource = resources.dstImage;
            } else {
                // Copy
                BlitJobUsedResources resources;
                result = foeGfxVkBlitImageRenderJob(
                    renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                    renderTargetColourImageResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    presentImageResource, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resources);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                renderTargetColourImageResource = resources.srcImage;
                presentImageResource = resources.dstImage;
            }

#ifdef EDITOR_MODE
            result = foeImGuiVkRenderUiJob(renderGraph, "RenderImGuiPass", VK_NULL_HANDLE,
                                           presentImageResource, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                           &imguiRenderer, &imguiState, frameIndex,
                                           &presentImageResource);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }
#endif
            // This is called so that the swapchain advances it's internal acquired index, as if it
            // was presented
            VkSwapchainKHR swapchain2;
            uint32_t index;
            window->swapchain.presentData(&swapchain2, &index);

            result = foeGfxVkPresentSwapchainImageRenderJob(renderGraph, "presentFinalImage",
                                                            frameData[frameIndex].frameComplete,
                                                            presentImageResource);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            result = foeGfxVkExecuteRenderGraph(renderGraph, gfxSession, gfxDelayedDestructor);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            foeGfxVkDestroyRenderGraph(renderGraph);

            // Set frame index data
            lastFrameIndex = frameIndex;
            frameIndex = -1;
        }
    SKIP_FRAME_RENDER:;

        foeWaitSyncThreads(threadPool);
    }
    FOE_LOG(General, Info, "Exiting main loop")

    return 0;
}