// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "application.hpp"

#include <FreeImage.h>
#include <GLFW/glfw3.h>
#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/job/blit_image.hpp>
#include <foe/graphics/vk/render_graph/job/copy_image.hpp>
#include <foe/graphics/vk/render_graph/job/export_image.hpp>
#include <foe/graphics/vk/render_graph/job/import_image.hpp>
#include <foe/graphics/vk/render_graph/job/present_image.hpp>
#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>
#include <foe/graphics/vk/render_pass_pool.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/render_view_pool.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/sample_count.h>
#include <foe/graphics/vk/session.h>
#include <foe/imex/exporters.h>
#include <foe/physics/system.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/quaternion_math.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/wsi/keyboard.hpp>
#include <foe/wsi/mouse.hpp>
#include <foe/wsi/vulkan.h>

#include "graphics.hpp"
#include "log.hpp"
#include "logging.hpp"
#include "register_basic_functionality.h"
#include "render_graph/render_scene.hpp"
#include "simulation/armature_system.hpp"
#include "simulation/position_descriptor_pool.hpp"
#include "simulation/type_defs.h"
#include "simulation/vk_animation.hpp"

#ifdef FOE_XR_SUPPORT
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
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error: {}", __FILE__, \
                __LINE__, buffer);                                                                 \
        return std::make_tuple(false, result.value);                                               \
    }

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        char buffer[FOE_MAX_RESULT_STRING_SIZE];                                                   \
        result.toString(result.value, buffer);                                                     \
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}", __FILE__,  \
                __LINE__, buffer);                                                                 \
        return result.value;                                                                       \
    }

#define END_PROGRAM_TUPLE                                                                          \
    {                                                                                              \
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{}", __FILE__, __LINE__);     \
        return std::make_tuple(false, 1);                                                          \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{}", __FILE__, __LINE__);     \
        return 1;                                                                                  \
    }

#include "state_import/import_state.hpp"

auto Application::initialize(int argc, char **argv) -> std::tuple<bool, int> {
    FreeImage_Initialise();

    foeResultSet result;

    initializeLogging();
    result = registerBasicFunctionality();
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Error registering basic functionality: {}",
                buffer)
        return std::make_tuple(false, result.value);
    }

    auto [continueRun, retVal] = loadSettings(argc, argv, settings, searchPaths);
    if (!continueRun) {
        return std::make_tuple(false, retVal);
    }

    result = foeCreateThreadPool(1, 1, &threadPool);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM_TUPLE

    result = importState("persistent", &searchPaths, &pSimulationSet);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Error importing '{}' state with error: {}",
                "persistent", buffer)

        return std::make_tuple(false, result.value);
    }

#ifdef EDITOR_MODE
    auto *pImGuiContext = ImGui::CreateContext();

    result = registerImGui(&imguiRegistrar);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM_TUPLE

    devConsole.registerWithLogger();

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

            it.position = glm::vec3{0.f, 0.f, -17.5f};
            it.orientation = glm::quat{1.f, 0.f, 0.f, 0.f};
            it.fovY = 60;
            it.nearZ = 2;
            it.farZ = 50;

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
        uint32_t maxSupportedCount = foeGfxVkGetSampleCount(globalMSAA);

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

    result = foeGfxCreateDelayedCaller(gfxSession, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1,
                                       &gfxDelayedDestructor);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM_TUPLE
    }

    result = foeGfxCreateRenderViewPool(gfxSession, 32, &gfxRenderViewPool);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM_TUPLE
    }

    for (auto &it : windowData) {
        result = foeGfxAllocateRenderView(gfxRenderViewPool, &it.renderView);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }
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
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL,
                    "XR support enabled but no HMD session was detected/started.")
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

#ifdef EDITOR_MODE
        imguiRenderer.deinitialize(gfxSession);
#endif
    }

    // Destroy window data
    for (auto &it : windowData) {
        it.renderView = FOE_NULL_HANDLE;

#ifdef EDITOR_MODE
        windowInfo.removeWindow(it.window);
#endif
        if (it.gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
            foeGfxDestroyRenderTarget(it.gfxOffscreenRenderTarget);
        it.gfxOffscreenRenderTarget = FOE_NULL_HANDLE;

        if (gfxSession != FOE_NULL_HANDLE)
            foeGfxVkDestroySwapchain(gfxSession, it.swapchain);

        if (it.surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(foeGfxVkGetRuntimeInstance(gfxRuntime), it.surface, nullptr);
        it.surface = VK_NULL_HANDLE;

        if (it.window != FOE_NULL_HANDLE)
            foeWsiDestroyWindow(it.window);
        it.window = FOE_NULL_HANDLE;
    }

    // Cleanup graphics
    if (gfxRenderViewPool != FOE_NULL_HANDLE)
        foeGfxDestroyRenderViewPool(gfxSession, gfxRenderViewPool);
    gfxRenderViewPool = FOE_NULL_HANDLE;

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
    devConsole.deregisterFromLogger();
#endif

    // Output configuration settings to a YAML configuration file
    // saveSettings(settings);

    FreeImage_DeInitialise();
}

namespace {

void processUserInput(double timeElapsedInSeconds,
                      foeWsiKeyboard const *pKeyboard,
                      foeWsiMouse const *pMouse,
                      glm::vec3 *pPosition,
                      glm::quat *pOrientation) {
    constexpr float movementMultiplier = 10.f;
    constexpr float rorationMultiplier = 40.f;
    float multiplier = timeElapsedInSeconds * 3.f; // 3 units per second

    if (pMouse->inWindow) {
        if (pKeyboard->keyDown(GLFW_KEY_Z)) { // Up
            *pPosition += upVec(*pOrientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_X)) { // Down
            *pPosition -= upVec(*pOrientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keyDown(GLFW_KEY_W)) { // Forward
            *pPosition += forwardVec(*pOrientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_S)) { // Back
            *pPosition -= forwardVec(*pOrientation) * movementMultiplier * multiplier;
        }

        if (pKeyboard->keyDown(GLFW_KEY_A)) { // Left
            *pPosition += leftVec(*pOrientation) * movementMultiplier * multiplier;
        }
        if (pKeyboard->keyDown(GLFW_KEY_D)) { // Right
            *pPosition -= leftVec(*pOrientation) * movementMultiplier * multiplier;
        }

        if (pMouse->buttonDown(GLFW_MOUSE_BUTTON_1)) {
            *pOrientation =
                changeYaw(*pOrientation, -glm::radians(pMouse->oldPosition.x - pMouse->position.x));
            *pOrientation = changePitch(*pOrientation,
                                        glm::radians(pMouse->oldPosition.y - pMouse->position.y));

            if (pKeyboard->keyDown(GLFW_KEY_Q)) { // Roll Left
                *pOrientation =
                    changeRoll(*pOrientation, glm::radians(rorationMultiplier * multiplier));
            }
            if (pKeyboard->keyDown(GLFW_KEY_E)) { // Roll Right
                *pOrientation =
                    changeRoll(*pOrientation, -glm::radians(rorationMultiplier * multiplier));
            }
        }
    }
}

} // namespace

foeResultSet Application::startXR(bool localPoll) {
    foeResultSet result{.value = FOE_SUCCESS, .toString = NULL};

    if (xrRuntime == FOE_NULL_HANDLE) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "Tried to start an XR session, but no XR runtime has been started");
    }
#ifdef FOE_XR_SUPPORT
    else {
        result = createXrSession(xrRuntime, gfxSession, &xrSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

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
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);

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
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

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
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

            goto START_XR_FAILED;
        }
        xrViews.clear();
        for (auto const &it : viewConfigs) {
            xrViews.emplace_back(foeXrVkSessionView{.viewConfig = it});
        }

        // OpenXR Swapchains
        uint32_t formatCount;
        std::vector<int64_t> swapchainFormats;
        result = foeOpenXrEnumerateSwapchainFormats(xrSession, &formatCount, nullptr);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

            goto START_XR_FAILED;
        }

        swapchainFormats.resize(formatCount);
        result =
            foeOpenXrEnumerateSwapchainFormats(xrSession, &formatCount, swapchainFormats.data());
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

            goto START_XR_FAILED;
        }

        for (auto &it : xrViews) {
            it.format = static_cast<VkFormat>(swapchainFormats[0]);
        }

        foeGfxVkRenderPassPool renderPassPool = foeGfxVkGetRenderPassPool(gfxSession);

        VkAttachmentDescription xrAttachmentDescription{
            .format = static_cast<VkFormat>(swapchainFormats[0]),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        };
        xrRenderPass = foeGfxVkGetRenderPass(renderPassPool, 1, &xrAttachmentDescription);

        std::array<VkAttachmentDescription, 2> xrOffscreenAttachmentDescription{
            VkAttachmentDescription{
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
            }};
        xrOffscreenRenderPass =
            foeGfxVkGetRenderPass(renderPassPool, xrOffscreenAttachmentDescription.size(),
                                  xrOffscreenAttachmentDescription.data());

        result = foeGfxCreateRenderViewPool(gfxSession, xrViews.size(), &xrRenderViewPool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

            goto START_XR_FAILED;
        }

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
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                        __FILE__, __LINE__, buffer);

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
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                        __FILE__, __LINE__, buffer);

                goto START_XR_FAILED;
            }

            // Images
            uint32_t imageCount = view.images.size();
            result = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, &imageCount, nullptr);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                        __FILE__, __LINE__, buffer);

                goto START_XR_FAILED;
            }

            view.images.resize(imageCount);
            result = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, &imageCount,
                                                         view.images.data());
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                        __FILE__, __LINE__, buffer);

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
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);

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
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);

                    goto START_XR_FAILED;
                }

                view.framebuffers.emplace_back(newFramebuffer);
            }

            result = foeGfxAllocateRenderView(xrRenderViewPool, &view.renderView);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                        __FILE__, __LINE__, buffer);

                goto START_XR_FAILED;
            }
        }

        result = foeOpenXrBeginSession(xrSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                    __FILE__, __LINE__, buffer);

            goto START_XR_FAILED;
        }

        FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Started new XR session {}",
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
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);
                    return result;
                }
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
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);
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
            for (auto it : view.framebuffers) {
                vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);
            }
            for (auto it : view.imageViews) {
                vkDestroyImageView(foeGfxVkGetDevice(gfxSession), it, nullptr);
            }
            if (view.swapchain != XR_NULL_HANDLE) {
                xrDestroySwapchain(view.swapchain);
            }
        }

        if (xrRenderViewPool != FOE_NULL_HANDLE)
            foeGfxDestroyRenderViewPool(gfxSession, xrRenderViewPool);
        xrRenderViewPool = FOE_NULL_HANDLE;

        while (foeOpenXrGetSessionState(xrSession) != XR_SESSION_STATE_EXITING) {
            if (localPoll) {
                result = foeXrProcessEvents(xrRuntime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",
                            __FILE__, __LINE__, buffer);
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

namespace {

void destroy_VkSemaphore(VkSemaphore semaphore, foeGfxSession session) {
    vkDestroySemaphore(foeGfxVkGetDevice(session), semaphore, nullptr);
}

} // namespace

int Application::mainloop() {
    uint64_t frame = 0;
    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock{std::chrono::nanoseconds{0}};
    foeResultSet result;

    uint32_t lastFrameIndex = UINT32_MAX;
    uint32_t frameIndex = UINT32_MAX;

    for (int i = windowData.size() - 1; i >= 0; --i) {
        foeWsiWindowShow(windowData[i].window);
    }
    programClock.update();
    simulationClock.externalTime(programClock.currentTime<std::chrono::nanoseconds>());

    FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Entering main loop")
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

        // Window Processing
        for (size_t i = 0; i < windowData.size(); ++i) {
            auto &window = windowData[i];

#ifdef EDITOR_MODE
            // Only the first/primary window supports ImGui interaction
            if (i == 0) {
                imguiRenderer.keyboardInput(foeWsiGetKeyboard(window.window));
                imguiRenderer.mouseInput(foeWsiGetMouse(window.window));
            }

            // If ImGui is capturing, don't pass inputs through from the first window
            if ((!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse()) ||
                i != 0)
#endif
            {
                processUserInput(timeElapsedInSec, foeWsiGetKeyboard(window.window),
                                 foeWsiGetMouse(window.window), &window.position,
                                 &window.orientation);
            }

            // Check if window was resized, and if so request associated swapchains to be rebuilt
            if (foeWsiWindowResized(window.window)) {
                window.needSwapchainRebuild = true;

#ifdef EDITOR_MODE
                // ImGui only follows primary/first window size
                if (i == 0) {
                    int width, height;
                    foeWsiWindowGetSize(window.window, &width, &height);
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
                static int currentFrameSleep = 0;
                int lastFrameTime = frameTime.lastFrameTime<std::chrono::nanoseconds>().count();

                if (lastFrameTime > 16 * 1000000) {
                    --currentFrameSleep;
                    if (currentFrameSleep < 0)
                        currentFrameSleep = 0;
                } else if (lastFrameTime < 16 * 1000000) {
                    ++currentFrameSleep;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(currentFrameSleep));
            }

            // Swapchain updates if necessary
            for (auto &it : windowData) {
                // If no window here, skip
                if (it.window == FOE_NULL_HANDLE)
                    continue;

                performWindowMaintenance(&it, gfxSession, gfxDelayedDestructor, globalMSAA,
                                         depthFormat);
            }

            // Acquire Target Presentation Images
            std::vector<WindowData *> windowRenderList;
            windowRenderList.reserve(windowData.size());

            for (auto &it : windowData) {
                result = vk_to_foeResult(
                    foeGfxVkAcquireSwapchainImage(gfxSession, it.swapchain, &it.acquiredImageData));
                if (result.value == VK_TIMEOUT || result.value == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (result.value == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchains
                    it.needSwapchainRebuild = true;
                } else if (result.value == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it.needSwapchainRebuild = true;
                    it.acquiredImage = true;
                    windowRenderList.emplace_back(&it);
                } else if (result.value) {
                    // Catastrophic error
                    ERRC_END_PROGRAM
                } else {
                    // No issues, add it to be rendered
                    windowRenderList.emplace_back(&it);
                    it.acquiredImage = true;
                }

                if (it.acquiredImage) {
                    glm::mat4 matrix = glm::perspectiveFov(
                        glm::radians(it.fovY), (float)it.acquiredImageData.extent.width,
                        (float)it.acquiredImageData.extent.height, it.nearZ, it.farZ);
                    matrix *= glm::mat4_cast(it.orientation) *
                              glm::translate(glm::mat4(1.f), it.position);

                    foeGfxUpdateRenderView(it.renderView, sizeof(glm::mat4), &matrix);
                }
            }
            if (windowRenderList.empty()) {
                goto SKIP_FRAME_RENDER;
            }

            foeGfxUpdateRenderViewPool(gfxSession, gfxRenderViewPool);

            frameTime.newFrame();

            // Run Systems that generate graphics data
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

                        glm::mat4 matrix = foeXrCameraProjectionMatrix(&xrViews[i].camera) *
                                           foeXrCameraViewMatrix(&xrViews[i].camera);

                        foeGfxUpdateRenderView(xrViews[i].renderView, sizeof(glm::mat4), &matrix);
                    }

                    foeGfxUpdateRenderViewPool(gfxSession, xrRenderViewPool);

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
                        foeGfxVkRenderGraphJob renderTargetColourImportJob;
                        foeGfxVkRenderGraphResource renderTargetDepthImageResource;
                        foeGfxVkRenderGraphJob renderTargetDepthImportJob;

                        result = foeGfxVkImportImageRenderJob(
                            renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                            foeGfxVkGetRenderTargetImage(renderTarget, 0),
                            foeGfxVkGetRenderTargetImageView(renderTarget, 0), it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource,
                            &renderTargetColourImportJob);
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
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetDepthImageResource,
                            &renderTargetDepthImportJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        // Create the timeline semaphore
                        VkSemaphoreTypeCreateInfo timelineCI{
                            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                            .pNext = nullptr,
                            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                            .initialValue = 0,
                        };

                        VkSemaphoreCreateInfo semaphoreCI{
                            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                            .pNext = &timelineCI,
                            .flags = 0,
                        };

                        VkSemaphore timelineSemaphore;
                        VkResult vkResult =
                            vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                              &timelineSemaphore);
                        if (vkResult != VK_SUCCESS) {
                            result = vk_to_foeResult(vkResult);
                            ERRC_END_PROGRAM
                        }

                        foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                                    (PFN_foeGfxDelayedCall)destroy_VkSemaphore,
                                                    (void *)timelineSemaphore);

                        foeGfxVkRenderGraphResource xrSwapchainImageResource;
                        foeGfxVkRenderGraphJob xrSwapchainImportJob;

                        result = foeOpenXrVkImportSwapchainImageRenderJob(
                            renderGraph, "importXrViewSwapchainImage", VK_NULL_HANDLE,
                            "importXrViewSwapchainImage", timelineSemaphore, it.swapchain,
                            it.images[newIndex].image, it.imageViews[newIndex], it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &xrSwapchainImageResource,
                            &xrSwapchainImportJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        VkDescriptorSet viewDescriptorSet =
                            foeGfxVkGetRenderViewDescriptorSet(xrRenderViewPool, it.renderView);

                        foeGfxVkRenderGraphJob xrRenderSceneJob;
                        result = renderSceneJob(
                            renderGraph, "render3dScene", VK_NULL_HANDLE,
                            renderTargetColourImageResource, 1, &renderTargetColourImportJob,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderTargetDepthImageResource, 1,
                            &renderTargetDepthImportJob, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            globalMSAA, pSimulationSet, viewDescriptorSet, &xrRenderSceneJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphJob xrResolveOrCopyJob;
                        if (foeGfxVkGetRenderTargetSamples(renderTarget) != VK_SAMPLE_COUNT_1_BIT) {
                            // Resolve
                            result = foeGfxVkResolveImageRenderJob(
                                renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource, 1, &xrRenderSceneJob,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource, 1,
                                &xrSwapchainImportJob, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                &xrResolveOrCopyJob);
                            if (result.value != FOE_SUCCESS) {
                                ERRC_END_PROGRAM
                            }
                        } else {
                            // Copy
                            result = foeGfxVkCopyImageRenderJob(
                                renderGraph, "copyRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource, 1, &xrRenderSceneJob,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource, 1,
                                &xrSwapchainImportJob, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                &xrResolveOrCopyJob);
                            if (result.value != FOE_SUCCESS) {
                                ERRC_END_PROGRAM
                            }
                        }

                        result = foeGfxVkExportImageRenderJob(
                            renderGraph, "exportPresentationImage", VK_NULL_HANDLE,
                            xrSwapchainImageResource, 1, &xrResolveOrCopyJob,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {});
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        result = foeGfxVkRenderGraphCompile(renderGraph);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        result = foeGfxVkRenderGraphExecute(renderGraph, gfxSession,
                                                            gfxDelayedDestructor);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        { // Wait for XR swapchain
                            XrSwapchainImageWaitInfo waitInfo{
                                .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
                                .timeout = 10000, // In nanoseconds (0.01 ms)
                            };

                            XrResult xrResult{XR_TIMEOUT_EXPIRED};

                            do {
                                xrResult = xrWaitSwapchainImage(it.swapchain, &waitInfo);
                            } while (xrResult == XR_TIMEOUT_EXPIRED);

                            if (xrResult != XR_SUCCESS && xrResult != XR_SESSION_LOSS_PENDING) {
                                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                                XrResultToString(xrResult, buffer);
                                FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                                        "xrWaitSwapchainImage failed: {}", buffer);
                            } else {
                                VkSemaphoreSignalInfo signalInfo{
                                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                                    .pNext = nullptr,
                                    .semaphore = timelineSemaphore,
                                    .value = 1,
                                };

                                vkSignalSemaphore(foeGfxVkGetDevice(gfxSession), &signalInfo);
                            }

                            XrSwapchainImageReleaseInfo releaseInfo{
                                .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
                            };

                            xrResult = xrReleaseSwapchainImage(it.swapchain, &releaseInfo);
                            if (xrResult != XR_SUCCESS) {
                                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                                XrResultToString(xrResult, buffer);
                                FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL,
                                        "xrReleaseSwapchainImage error: {}", buffer)
                            }
                        }

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

            // Render Graph for this graphics tick
            foeGfxVkRenderGraph renderGraph;
            result = foeGfxVkCreateRenderGraph(&renderGraph);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            struct PresentSwapchain {
                foeGfxVkRenderGraphResource swapchain;
                std::vector<foeGfxVkRenderGraphJob> upstreamJobs;
            };

            std::vector<PresentSwapchain> swapchainPresentations;

            for (size_t i = 0; i < windowRenderList.size(); ++i) {
                auto &window = windowRenderList[i];

                result = foeGfxAcquireNextRenderTarget(window->gfxOffscreenRenderTarget,
                                                       FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                foeGfxVkRenderGraphResource renderTargetColourImageResource;
                foeGfxVkRenderGraphJob renderTargetColourImportJob;
                foeGfxVkRenderGraphResource renderTargetDepthImageResource;
                foeGfxVkRenderGraphJob renderTargetDepthImportJob;

                result = foeGfxVkImportImageRenderJob(
                    renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                    foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 0),
                    foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 0),
                    window->surfaceFormat.format, window->acquiredImageData.extent,
                    VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource,
                    &renderTargetColourImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                result = foeGfxVkImportImageRenderJob(
                    renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                    "renderTargetDepthImage",
                    foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 1),
                    foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 1),
                    depthFormat, window->acquiredImageData.extent, VK_IMAGE_LAYOUT_UNDEFINED, true,
                    {}, &renderTargetDepthImageResource, &renderTargetDepthImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                VkDescriptorSet cameraProjViewDescriptor =
                    foeGfxVkGetRenderViewDescriptorSet(gfxRenderViewPool, window->renderView);

                foeGfxVkRenderGraphJob renderSceneJobHandle;
                result = renderSceneJob(
                    renderGraph, "render3dScene", VK_NULL_HANDLE, renderTargetColourImageResource,
                    1, &renderTargetColourImportJob, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    renderTargetDepthImageResource, 1, &renderTargetDepthImportJob,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, globalMSAA, pSimulationSet,
                    cameraProjViewDescriptor, &renderSceneJobHandle);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                foeGfxVkRenderGraphResource presentImageResource;
                foeGfxVkRenderGraphJob presentImageImportJob;

                result = foeGfxVkImportSwapchainImageRenderJob(
                    renderGraph, "importPresentationImage", VK_NULL_HANDLE, "presentImage",
                    window->acquiredImageData.swapchain, window->acquiredImageData.imageIndex,
                    window->acquiredImageData.image, window->acquiredImageData.view,
                    window->surfaceFormat.format, window->acquiredImageData.extent,
                    VK_IMAGE_LAYOUT_UNDEFINED, window->acquiredImageData.readySemaphore,
                    &presentImageResource, &presentImageImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                foeGfxVkRenderGraphJob resolveOrCopyJob;
                if (foeGfxVkGetRenderTargetSamples(window->gfxOffscreenRenderTarget) !=
                    VK_SAMPLE_COUNT_1_BIT) {
                    // Resolve
                    result = foeGfxVkResolveImageRenderJob(
                        renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                        renderTargetColourImageResource, 1, &renderSceneJobHandle,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, presentImageResource, 1,
                        &presentImageImportJob, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resolveOrCopyJob);
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }
                } else {
                    // Copy
                    result = foeGfxVkCopyImageRenderJob(
                        renderGraph, "copyRenderedImageToBackbuffer", VK_NULL_HANDLE,
                        renderTargetColourImageResource, 1, &renderSceneJobHandle,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, presentImageResource, 1,
                        &presentImageImportJob, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resolveOrCopyJob);
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }
                }

                static struct {
                    VmaAllocation alloc;
                    VkImage image;
                    VkFormat format;

                    VkExtent3D extent;
                    VkFence fence;
                    bool saved;
                } cpuImage = {};

                if (frame == 100 && i == 0) {
                    auto extent = window->acquiredImageData.extent;

                    // Create Fence
                    VkFenceCreateInfo fenceCI{
                        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    };

                    VkResult vkRes = vkCreateFence(foeGfxVkGetDevice(gfxSession), &fenceCI, nullptr,
                                                   &cpuImage.fence);
                    if (vkRes != VK_SUCCESS) {
                        std::abort();
                    }

                    // Create Image
                    cpuImage.format = VK_FORMAT_B8G8R8A8_UNORM;
                    cpuImage.extent = VkExtent3D{
                        .width = extent.width,
                        .height = extent.height,
                        .depth = 1U,
                    };

                    VkImageCreateInfo imageCI{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = cpuImage.format,
                        .extent = cpuImage.extent,
                        .mipLevels = 1U,
                        .arrayLayers = 1U,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .tiling = VK_IMAGE_TILING_LINEAR,
                        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    };

                    VmaAllocationCreateInfo allocCI{
                        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
                    };

                    vkRes = vmaCreateImage(foeGfxVkGetAllocator(gfxSession), &imageCI, &allocCI,
                                           &cpuImage.image, &cpuImage.alloc, nullptr);
                    if (vkRes != VK_SUCCESS) {
                        std::abort();
                    }

                    foeGfxVkRenderGraphResource cpuCopiedImage;
                    foeGfxVkRenderGraphJob cpuImageImportJob;
                    result = foeGfxVkImportImageRenderJob(
                        renderGraph, "importCpuImage", VK_NULL_HANDLE, "cpuImage", cpuImage.image,
                        VK_NULL_HANDLE, cpuImage.format, extent, VK_IMAGE_LAYOUT_UNDEFINED, true,
                        {}, &cpuCopiedImage, &cpuImageImportJob);
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }

                    foeGfxVkRenderGraphJob cpuResolveOrCopyJob;
                    if (foeGfxVkGetRenderTargetSamples(window->gfxOffscreenRenderTarget) !=
                        VK_SAMPLE_COUNT_1_BIT) {
                        // Resolve
                        result = foeGfxVkResolveImageRenderJob(
                            renderGraph, "resolveRenderedImageToCpuImage", VK_NULL_HANDLE,
                            renderTargetColourImageResource, 1, &renderSceneJobHandle,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cpuCopiedImage, 1,
                            &cpuImageImportJob, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            &cpuResolveOrCopyJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }
                    } else {
                        // Copy
                        result = foeGfxVkBlitImageRenderJob(
                            renderGraph, "blitRenderedImageToCpuImage", VK_NULL_HANDLE,
                            renderTargetColourImageResource, 1, &renderSceneJobHandle,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cpuCopiedImage, 1,
                            &cpuImageImportJob, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_FILTER_NEAREST, &cpuResolveOrCopyJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }
                    }

                    foeGfxVkExportImageRenderJob(renderGraph, "exportCpuImage", cpuImage.fence,
                                                 cpuCopiedImage, 1, &cpuResolveOrCopyJob,
                                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {});
                }

                if (cpuImage.fence != VK_NULL_HANDLE && !cpuImage.saved && i == 0) {
                    VkResult vkRes =
                        vkGetFenceStatus(foeGfxVkGetDevice(gfxSession), cpuImage.fence);
                    if (vkRes == VK_SUCCESS) {
                        cpuImage.saved = true;

                        FIBITMAP *pBitmap =
                            FreeImage_Allocate(cpuImage.extent.width, cpuImage.extent.height, 32);

                        auto *pBitmapData = (void *)FreeImage_GetBits(pBitmap);
                        void *pData = nullptr;
                        VkResult vkRes =
                            vmaMapMemory(foeGfxVkGetAllocator(gfxSession), cpuImage.alloc, &pData);
                        if (vkRes != VK_SUCCESS) {
                            std::abort();
                        }

                        memcpy(pBitmapData, pData,
                               cpuImage.extent.width * cpuImage.extent.height * 4);
                        vmaUnmapMemory(foeGfxVkGetAllocator(gfxSession), cpuImage.alloc);

                        FreeImage_Save(FIF_PNG, pBitmap, "test.png");

                        FreeImage_Unload(pBitmap);

                        vkDestroyFence(foeGfxVkGetDevice(gfxSession), cpuImage.fence, nullptr);

                        vmaDestroyImage(foeGfxVkGetAllocator(gfxSession), cpuImage.image,
                                        cpuImage.alloc);

                        FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "SAVED IMAGE");
                    }
                }

                foeGfxVkRenderGraphJob renderDebugUiJob = FOE_NULL_HANDLE;
#ifdef EDITOR_MODE
                // ImGui only renders on the first/primary window
                if (window == &windowData[0]) {
                    result = foeImGuiVkRenderUiJob(renderGraph, "RenderImGuiPass", VK_NULL_HANDLE,
                                                   presentImageResource, 1, &resolveOrCopyJob,
                                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &imguiRenderer,
                                                   &imguiState, frameIndex, &renderDebugUiJob);
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }
                }
#endif

                window->acquiredImage = false;

                swapchainPresentations.emplace_back(PresentSwapchain{
                    .swapchain = presentImageResource,
                    .upstreamJobs = {(renderDebugUiJob != FOE_NULL_HANDLE) ? renderDebugUiJob
                                                                           : resolveOrCopyJob},
                });
            }

            // Need to collate the presentations together for the frameComplete fence
            if (!swapchainPresentations.empty()) {
                std::vector<foeGfxVkSwapchainPresentInfo> presentInfo;

                for (auto &it : swapchainPresentations) {
                    presentInfo.emplace_back(foeGfxVkSwapchainPresentInfo{
                        .swapchainResource = it.swapchain,
                        .upstreamJobCount = (uint32_t)it.upstreamJobs.size(),
                        .pUpstreamJobs = it.upstreamJobs.data(),
                    });
                }

                result = foeGfxVkPresentSwapchainImageRenderJob(
                    renderGraph, "presentFinalImage", frameData[frameIndex].frameComplete,
                    presentInfo.size(), presentInfo.data());
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }
            }

            result = foeGfxVkRenderGraphCompile(renderGraph);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            result = foeGfxVkRenderGraphExecute(renderGraph, gfxSession, gfxDelayedDestructor);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            foeGfxVkDestroyRenderGraph(renderGraph);

            // Set frame index data
            lastFrameIndex = frameIndex;
            frameIndex = -1;
            ++frame;
        }

    SKIP_FRAME_RENDER:;

        foeWaitSyncThreads(threadPool);
    }
    FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Exiting main loop")

    return 0;
}