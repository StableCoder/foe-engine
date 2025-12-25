// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "application.hpp"

#include <MagickCore/MagickCore.h>
#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/job/blit_image.hpp>
#include <foe/graphics/vk/render_graph/job/copy_image.hpp>
#include <foe/graphics/vk/render_graph/job/export_image.hpp>
#include <foe/graphics/vk/render_graph/job/import_image.hpp>
#include <foe/graphics/vk/render_graph/job/present_image.hpp>
#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>
#include <foe/graphics/vk/render_graph/job/synchronize.hpp>
#include <foe/graphics/vk/render_pass_pool.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/render_view_pool.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/sample_count.h>
#include <foe/graphics/vk/session.h>
#include <foe/imex/exporters.h>
#include <foe/physics/system.h>
#include <foe/physics/type_defs.h>
#include <foe/quaternion_math.hpp>
#include <foe/simulation/simulation.hpp>

#include "graphics.hpp"
#include "log.hpp"
#include "logging.hpp"
#include "register_basic_functionality.h"
#include "render_graph/render_scene.hpp"
#include "simulation/animated_bone_system.h"
#include "simulation/armature_state.h"
#include "simulation/render_system.hpp"
#include "simulation/type_defs.h"
#include "vk_result.h"
#include "wsi_glfw/window.hpp"

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/runtime.h>
#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include "xr.hpp"
#include "xr_result.h"
#endif

#ifdef EDITOR_MODE
#include <foe/imgui/vk/render_graph_job_imgui.hpp>

#include "imgui/register.hpp"
#include "wsi_glfw/imgui.hpp"
#endif

#include <fstream>
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

    {
        // window creation
        for (auto const &it : settings.windows) {
            std::unique_ptr<GLFW_WindowData> pNewWindow{new GLFW_WindowData};

            bool result = createGlfwWindow(it.width, it.height, it.title.c_str(), pNewWindow.get());
            if (!result)
                std::abort();

            pNewWindow->vsync = it.vsync;

            pNewWindow->position = glm::vec3{0.f, 0.f, -17.5f};
            pNewWindow->orientation = glm::quat{1.f, 0.f, 0.f, 0.f};
            pNewWindow->fovY = 60;
            pNewWindow->nearZ = 2;
            pNewWindow->farZ = 50;

#ifdef EDITOR_MODE
            imguiAddGlfwWindow(&windowInfo, pNewWindow->pWindow, &pNewWindow->keyboard,
                               &pNewWindow->mouse);
#endif

            windowData.emplace_back(pNewWindow.release());
        }

#ifdef FOE_XR_SUPPORT
        if (settings.xr.enableXr || settings.xr.forceXr) {
            result = createXrRuntime(settings.xr.debugLogging, &xrRuntime);
            if (result.value != FOE_SUCCESS && settings.xr.forceXr) {
                ERRC_END_PROGRAM_TUPLE
            }
        }
#endif

        std::vector<std::string> vkInstanceExtensions;

        { // wsi - extensions
            uint32_t extensionCount;
            char const *const *ppExtensionNames;

            if (!getGlfwVkInstanceExtensions(&extensionCount, &ppExtensionNames))
                std::abort();
            for (uint32_t i = 0; i < extensionCount; ++i) {
                vkInstanceExtensions.emplace_back(ppExtensionNames[i]);
            }
        }

        result =
            createGfxRuntime(xrRuntime, settings.graphics.validation,
                             settings.graphics.debugLogging, {}, vkInstanceExtensions, &gfxRuntime);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }

        for (auto it : windowData) {
            if (!createGlfwWindowVkSurface(gfxRuntime, it, nullptr, &it->surface))
                std::abort();
        }

        std::vector<VkSurfaceKHR> surfaces;
        for (auto const it : windowData) {
            surfaces.push_back(it->surface);
        }

        result = createGfxSession(
            gfxRuntime, xrRuntime, settings.general.enableWindows || !windowData.empty(),
            std::move(surfaces), settings.graphics.gpu, settings.xr.forceXr, &gfxSession);
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
        result = foeGfxAllocateRenderView(gfxRenderViewPool, &it->renderView);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM_TUPLE
        }
    }

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

        foeResourcePoolSetAsyncTaskCallback(pSimulationSet->resourcePool, (void *)threadPool,
                                            asyncTaskFunc);
    }

#ifdef FOE_XR_SUPPORT
    if (settings.xr.enableXr || settings.xr.forceXr) {
        result = ::startXR(xrRuntime, gfxSession, gfxDelayedDestructor, depthFormat, globalMSAA,
                           true, &xrData);

        // If the user specified to force an XR session and couldn't find/create the
        // session, fail out
        if (settings.xr.forceXr && xrData.session == FOE_NULL_HANDLE) {
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
    stopXR(xrRuntime, gfxSession, true, &xrData);

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
    for (auto it : windowData) {
#ifdef EDITOR_MODE
        windowInfo.removeWindow(it->pWindow);
#endif
        destroyGlfwWindow(gfxRuntime, gfxSession, it);
        delete it;
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

    programClock.update();
    simulationClock.externalTime(programClock.currentTime<std::chrono::nanoseconds>());

    FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Entering main loop")
    while (!windowData.empty()
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
                if (xrData.session == FOE_NULL_HANDLE) {
                    foeScheduleAsyncTask(
                        threadPool,
                        [](void *pUserData) {
                            Application *pApplication = (Application *)pUserData;
                            startXR(pApplication->xrRuntime, pApplication->gfxSession,
                                    pApplication->gfxDelayedDestructor, pApplication->depthFormat,
                                    pApplication->globalMSAA, false, &pApplication->xrData);
                        },
                        this);
                } else {
                    foeScheduleAsyncTask(
                        threadPool,
                        [](void *pUserData) {
                            Application *pApplication = (Application *)pUserData;
                            stopXR(pApplication->xrRuntime, pApplication->gfxSession, false,
                                   &pApplication->xrData);
                        },
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
        foeProcessAnimatedBoneSystem(
            (foeAnimatedBoneSystem)foeSimulationGetSystem(
                pSimulationSet, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM),
            timeElapsedInSec);

        foePhysicsProcessSystem((foePhysicsSystem)foeSimulationGetSystem(
                                    pSimulationSet, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM),
                                timeElapsedInSec);

        foeProcessRenderSystem((foeRenderSystem)foeSimulationGetSystem(
            pSimulationSet, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM));

        // Process Window Events
        for (auto &it : windowData) {
            it->mouse.preprocessing();
            it->keyboard.preprocessing();
            it->resized = false;
        }
        processGlfwEvents();

#ifdef FOE_XR_SUPPORT
        // Process XR Events
        if (xrRuntime != FOE_NULL_HANDLE)
            foeXrProcessEvents(xrRuntime);
#endif

        // Window Processing
        for (auto it = windowData.begin(); it != windowData.end();) {
            GLFW_WindowData *window = *it;

            // if window is set to close, destroy it now
            if (window->requestClose) {
#ifdef EDITOR_MODE
                windowInfo.removeWindow(window->pWindow);
#endif
                destroyGlfwWindow(gfxRuntime, gfxSession, window);
                delete window;

                it = windowData.erase(it);
                continue;
            }

#ifdef EDITOR_MODE
            // Only the first/primary window supports ImGui interaction
            if (it == windowData.begin()) {
                std::vector<uint32_t> pressedKeycodes;
                std::vector<uint32_t> pressedScancodes;
                size_t const pressedCount = window->keyboard.pressedCodes.size();
                pressedKeycodes.reserve(pressedCount);
                pressedScancodes.reserve(pressedCount);
                for (size_t i = 0; i < pressedCount; ++i) {
                    auto const &it = window->keyboard.pressedCodes[i];

                    pressedKeycodes.emplace_back(it.keycode);
                    pressedScancodes.emplace_back(it.scancode);
                }

                std::vector<uint32_t> releasedKeycodes;
                std::vector<uint32_t> releasedScancodes;
                size_t const releasedCount = window->keyboard.releasedCodes.size();
                releasedKeycodes.reserve(releasedCount);
                releasedScancodes.reserve(releasedCount);
                for (size_t i = 0; i < releasedCount; ++i) {
                    auto const &it = window->keyboard.releasedCodes[i];

                    releasedKeycodes.emplace_back(it.keycode);
                    releasedScancodes.emplace_back(it.scancode);
                }

                imguiRenderer.keyboardInput(window->keyboard.unicodeChar, imguiGlfwKeyConvert,
                                            pressedKeycodes.data(), pressedScancodes.data(),
                                            pressedKeycodes.size(), releasedKeycodes.data(),
                                            releasedScancodes.data(), releasedKeycodes.size());

                std::vector<uint32_t> buttonsPressed{window->mouse.pressedButtons.cbegin(),
                                                     window->mouse.pressedButtons.cend()};
                std::vector<uint32_t> buttonsReleased{window->mouse.releasedButtons.cbegin(),
                                                      window->mouse.releasedButtons.cend()};
                imguiRenderer.mouseInput(window->mouse.position.x, window->mouse.position.y,
                                         window->mouse.scroll.x, window->mouse.scroll.y,
                                         buttonsPressed.data(), buttonsPressed.size(),
                                         buttonsReleased.data(), buttonsReleased.size());
            }

            // If ImGui is capturing, don't pass inputs through from the first window
            if ((!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse()) ||
                it != windowData.begin())
#endif
            {
                processUserInput(window, timeElapsedInSec);
            }

            // Check if window was resized, and if so request associated swapchains to
            // be rebuilt
            if (window->resized) {
                window->needSwapchainRebuild = true;
            }

#ifdef EDITOR_MODE
            // ImGui only follows primary/first window size
            if (it == windowData.begin()) {
                int width, height;
                getGlfwWindowLogicalSize(window, &width, &height);
                imguiRenderer.resize(width, height);
                float xScale, yScale;
                getGlfwWindowScale(window, &xScale, &yScale);
                imguiRenderer.rescale(xScale, yScale);
            }
#endif

            ++it;
        }

        // Determine if the next frame is available to start rendering to, if we
        // don't have one
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

        // If we have a frame we can render to, proceed to check for ready-to-render
        // data
        if (frameIndex != UINT32_MAX) {
#ifdef FOE_XR_SUPPORT
            // Lock rendering to OpenXR framerate, which overrides regular rendering
            bool xrAcquiredFrame = false;
            if (xrData.session != FOE_NULL_HANDLE && foeOpenXrGetSessionActive(xrData.session)) {
                xrAcquiredFrame = true;

                XrFrameWaitInfo frameWaitInfo{.type = XR_TYPE_FRAME_WAIT_INFO};
                xrData.frameState = XrFrameState{.type = XR_TYPE_FRAME_STATE};
                result = xr_to_foeResult(xrWaitFrame(foeOpenXrGetSession(xrData.session),
                                                     &frameWaitInfo, &xrData.frameState));
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
            for (auto it : windowData) {
                // If no window here, skip
                if (it->pWindow == FOE_NULL_HANDLE)
                    continue;

                result = performGlfwWindowMaintenance(it, gfxSession, gfxDelayedDestructor,
                                                      globalMSAA, depthFormat);
                if (result.value != FOE_SUCCESS)
                    ERRC_END_PROGRAM
            }

            // Acquire Target Presentation Images
            std::vector<GLFW_WindowData *> windowRenderList;

            for (auto &it : windowData) {
                result = vk_to_foeResult(foeGfxVkAcquireSwapchainImage(gfxSession, it->swapchain,
                                                                       &it->acquiredImageData));
                if (result.value == VK_TIMEOUT || result.value == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (result.value == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchain
                    it->needSwapchainRebuild = true;
                } else if (result.value == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it->needSwapchainRebuild = true;
                    it->acquiredImage = true;
                    windowRenderList.emplace_back(it);
                } else if (result.value) {
                    // Catastrophic error
                    ERRC_END_PROGRAM
                } else {
                    // No issues, add it to be rendered
                    windowRenderList.emplace_back(it);
                    it->acquiredImage = true;
                }

                if (it->acquiredImage) {
                    glm::mat4 matrix = glm::perspectiveFov(
                        glm::radians(it->fovY), (float)it->acquiredImageData.extent.width,
                        (float)it->acquiredImageData.extent.height, it->nearZ, it->farZ);
                    matrix *= glm::mat4_cast(it->orientation) *
                              glm::translate(glm::mat4(1.f), it->position);

                    foeGfxUpdateRenderView(it->renderView, sizeof(glm::mat4), &matrix);
                }
            }
            if (windowRenderList.empty()) {
                goto SKIP_FRAME_RENDER;
            }

            foeGfxUpdateRenderViewPool(gfxSession, gfxRenderViewPool);

            frameTime.newFrame();

            // Run Systems that generate graphics data
            foeProcessRenderSystemGraphics(
                (foeRenderSystem)foeSimulationGetSystem(
                    pSimulationSet, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM),
                frameIndex);

#ifdef FOE_XR_SUPPORT
            // OpenXR Render Section
            std::vector<XrCompositionLayerProjectionView> projectionViews{
                xrData.views.size(), XrCompositionLayerProjectionView{
                                         .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}};

            if (xrAcquiredFrame) {
                XrFrameBeginInfo frameBeginInfo{.type = XR_TYPE_FRAME_BEGIN_INFO};
                result = xr_to_foeResult(
                    xrBeginFrame(foeOpenXrGetSession(xrData.session), &frameBeginInfo));
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                if (xrData.frameState.shouldRender) {
                    XrViewLocateInfo viewLocateInfo{
                        .type = XR_TYPE_VIEW_LOCATE_INFO,
                        .viewConfigurationType = foeOpenXrGetViewConfigurationType(xrData.session),
                        .displayTime = xrData.frameState.predictedDisplayTime,
                        .space = foeOpenXrGetSpace(xrData.session),
                    };
                    XrViewState viewState{.type = XR_TYPE_VIEW_STATE};
                    std::vector<XrView> views{xrData.views.size(), XrView{.type = XR_TYPE_VIEW}};
                    uint32_t viewCountOutput;
                    result = xr_to_foeResult(
                        xrLocateViews(foeOpenXrGetSession(xrData.session), &viewLocateInfo,
                                      &viewState, views.size(), &viewCountOutput, views.data()));
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }

                    for (size_t i = 0; i < views.size(); ++i) {
                        projectionViews[i].pose = views[i].pose;
                        projectionViews[i].fov = views[i].fov;

                        xrData.views[i].camera.nearZ = 1;
                        xrData.views[i].camera.farZ = 100;

                        xrData.views[i].camera.fov = views[i].fov;
                        xrData.views[i].camera.pose = views[i].pose;

                        glm::mat4 matrix = foeXrCameraProjectionMatrix(&xrData.views[i].camera) *
                                           foeXrCameraViewMatrix(&xrData.views[i].camera);

                        foeGfxUpdateRenderView(xrData.views[i].renderView, sizeof(glm::mat4),
                                               &matrix);
                    }

                    foeGfxUpdateRenderViewPool(gfxSession, xrData.renderViewPool);

                    // Render Code
                    for (size_t i = 0; i < xrData.views.size(); ++i) {
                        auto &it = xrData.views[i];

                        result = foeGfxAcquireNextRenderTarget(it.offscreenRenderTarget,
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
                            foeGfxVkGetRenderTargetImage(it.offscreenRenderTarget, 0),
                            foeGfxVkGetRenderTargetImageView(it.offscreenRenderTarget, 0),
                            xrData.colourFormat,
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
                            "renderTargetDepthImage",
                            foeGfxVkGetRenderTargetImage(it.offscreenRenderTarget, 1),
                            foeGfxVkGetRenderTargetImageView(it.offscreenRenderTarget, 1),
                            depthFormat,
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

                        VkResult vkResult =
                            vkCreateSemaphore(foeGfxVkGetDevice(gfxSession), &semaphoreCI, nullptr,
                                              &it.timelineSemaphore);
                        if (vkResult != VK_SUCCESS) {
                            result = vk_to_foeResult(vkResult);
                            ERRC_END_PROGRAM
                        }

                        foeGfxAddDefaultDelayedCall(gfxDelayedDestructor,
                                                    (PFN_foeGfxDelayedCall)destroy_VkSemaphore,
                                                    (void *)it.timelineSemaphore);

                        foeGfxVkRenderGraphResource xrSwapchainImageResource;
                        foeGfxVkRenderGraphJob xrSwapchainImportJob;

                        result = foeOpenXrVkImportSwapchainImageRenderJob(
                            renderGraph, "importxrData.viewswapchainImage", VK_NULL_HANDLE,
                            "importxrData.viewswapchainImage", it.timelineSemaphore, it.swapchain,
                            it.images[newIndex].image, it.imageViews[newIndex], xrData.colourFormat,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &xrSwapchainImageResource,
                            &xrSwapchainImportJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        VkDescriptorSet viewDescriptorSet = foeGfxVkGetRenderViewDescriptorSet(
                            xrData.renderViewPool, it.renderView);

                        foeGfxVkRenderGraphJob xrRenderSceneJob;
                        result = renderSceneJob(
                            renderGraph, "render3dScene", VK_NULL_HANDLE,
                            renderTargetColourImageResource, 1, &renderTargetColourImportJob,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderTargetDepthImageResource, 1,
                            &renderTargetDepthImportJob, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            globalMSAA, pSimulationSet, viewDescriptorSet, frameIndex,
                            &xrRenderSceneJob);
                        if (result.value != FOE_SUCCESS) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphJob xrResolveOrCopyJob;
                        if (foeGfxVkGetRenderTargetSamples(it.offscreenRenderTarget) !=
                            VK_SAMPLE_COUNT_1_BIT) {
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
                                    .semaphore = it.timelineSemaphore,
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
                }
            }
#endif

            // Render Graph for this graphics tick
            foeGfxVkRenderGraph renderGraph;
            result = foeGfxVkCreateRenderGraph(&renderGraph);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            std::vector<foeGfxVkRenderGraphJob> completeJobList;

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
                    cameraProjViewDescriptor, frameIndex, &renderSceneJobHandle);
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

                        MagickCoreGenesis(nullptr, MagickFalse);

                        ExceptionInfo *exceptionInfo;
                        exceptionInfo = AcquireExceptionInfo();

                        void *pData = nullptr;
                        VkResult vkRes =
                            vmaMapMemory(foeGfxVkGetAllocator(gfxSession), cpuImage.alloc, &pData);
                        if (vkRes != VK_SUCCESS) {
                            std::abort();
                        }

                        Image *image =
                            ConstituteImage(cpuImage.extent.width, cpuImage.extent.height, "BGRA",
                                            CharPixel, pData, exceptionInfo);

                        vmaUnmapMemory(foeGfxVkGetAllocator(gfxSession), cpuImage.alloc);

                        vkDestroyFence(foeGfxVkGetDevice(gfxSession), cpuImage.fence, nullptr);

                        vmaDestroyImage(foeGfxVkGetAllocator(gfxSession), cpuImage.image,
                                        cpuImage.alloc);

                        ImageInfo *imageInfo = AcquireImageInfo();
                        strcpy(imageInfo->filename, "test.png");

                        size_t blobSize = 0;
                        void *blob = ImageToBlob(imageInfo, image, &blobSize, exceptionInfo);

                        std::ofstream outFile("test.png",
                                              std::ofstream::out | std::ofstream::binary);
                        outFile.write((char const *)blob, blobSize);

                        image = DestroyImage(image);
                        imageInfo = DestroyImageInfo(imageInfo);

                        exceptionInfo = DestroyExceptionInfo(exceptionInfo);

                        MagickCoreTerminus();

                        FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "SAVED IMAGE");
                    }
                }

                foeGfxVkRenderGraphJob renderDebugUiJob = FOE_NULL_HANDLE;
#ifdef EDITOR_MODE
                // ImGui only renders on the first/primary window
                if (window == windowData[0]) {
                    result = foeImGuiVkRenderUiJob(renderGraph, "RenderImGuiPass", VK_NULL_HANDLE,
                                                   presentImageResource, 1, &resolveOrCopyJob,
                                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &imguiRenderer,
                                                   &imguiState, frameIndex, &renderDebugUiJob);
                    if (result.value != FOE_SUCCESS) {
                        ERRC_END_PROGRAM
                    }
                }
#endif
                // For the 'frame complete' fence, use the jobs just before swapchain
                // present.
                completeJobList.emplace_back(
                    (renderDebugUiJob != FOE_NULL_HANDLE) ? renderDebugUiJob : resolveOrCopyJob);

                window->acquiredImage = false;

                foeGfxVkSwapchainPresentInfo presentInfo = {
                    .swapchainResource = presentImageResource,
                    .upstreamJobCount = 1U,
                    .pUpstreamJobs = (renderDebugUiJob != FOE_NULL_HANDLE) ? &renderDebugUiJob
                                                                           : &resolveOrCopyJob,
                };

                result = foeGfxVkPresentSwapchainImageRenderJob(renderGraph, "presentFinalImage",
                                                                FOE_NULL_HANDLE, 1U, &presentInfo);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }
            }

            // Create a single final frame-complete synchronization job for the fence
            // marking the end of a frame
            foeGfxVkRenderGraphJob frameCompleteSyncJob = FOE_NULL_HANDLE;
            result = foeGfxVkSynchronizeJob(
                renderGraph, "frameCompleteSynchronize", true, frameData[frameIndex].frameComplete,
                completeJobList.size(), completeJobList.data(), &frameCompleteSyncJob);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            result = foeGfxVkRenderGraphCompile(renderGraph);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

            result = foeGfxVkRenderGraphExecute(renderGraph, gfxSession, gfxDelayedDestructor);
            if (result.value != FOE_SUCCESS) {
                ERRC_END_PROGRAM
            }

#ifdef FOE_XR_SUPPORT
            if (xrAcquiredFrame) {
                XrCompositionLayerProjection layerProj;
                XrCompositionLayerBaseHeader *pLayers = NULL;

                if (xrData.frameState.shouldRender) {
                    // Assemble composition layers
                    layerProj = XrCompositionLayerProjection{
                        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
                        .layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT,
                        .space = foeOpenXrGetSpace(xrData.session),
                        .viewCount = static_cast<uint32_t>(projectionViews.size()),
                        .views = projectionViews.data(),
                    };

                    pLayers = reinterpret_cast<XrCompositionLayerBaseHeader *>(&layerProj);
                }

                XrFrameEndInfo endFrameInfo{
                    .type = XR_TYPE_FRAME_END_INFO,
                    .displayTime = xrData.frameState.predictedDisplayTime,
                    .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
                    .layerCount = (pLayers) ? 1U : 0U,
                    .layers = &pLayers,
                };
                result =
                    xr_to_foeResult(xrEndFrame(foeOpenXrGetSession(xrData.session), &endFrameInfo));
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }
            }
#endif

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