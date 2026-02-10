// Copyright (C) 2021-2026 George Cave.
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
#include "render_to_file.hpp"
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
#include "wsi_sdl3/imgui.hpp"
#endif

#include <fstream>
#include <thread>

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        char buffer[FOE_MAX_RESULT_STRING_SIZE];                                                   \
        result.toString(result.value, buffer);                                                     \
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "End called from {}:{} with error {}",         \
                __FILE__, __LINE__, buffer);                                                       \
        return result.value;                                                                       \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "End called from {}:{}", __FILE__, __LINE__);  \
        return 1;                                                                                  \
    }

#include "state_import/import_state.hpp"

int Application::initialize(int argc, char **argv) {
    MagickCoreGenesis(nullptr, MagickFalse);
    foeResultSet result;

    initializeLogging();
    result = registerBasicFunctionality();
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "Error registering basic functionality: {}",
                buffer)
        return result.value;
    }

    auto [continueRun, retVal] = loadSettings(argc, argv, settings, searchPaths);
    if (!continueRun) {
        return retVal;
    }

    result = foeCreateThreadPool(1, 1, &threadPool);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM

    result = importState("persistent", &searchPaths, &pSimulationSet);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "Error importing '{}' state with error: {}",
                "persistent", buffer)

        return result.value;
    }

#ifdef EDITOR_MODE
    auto *pImGuiContext = ImGui::CreateContext();

    result = registerImGui(&imguiRegistrar);
    if (result.value != FOE_SUCCESS)
        ERRC_END_PROGRAM

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
            if (it.implementation == Settings::Window::Implementation::GLFW) {
                std::unique_ptr<GLFW_WindowData> pNewWindow{new GLFW_WindowData};

                bool result =
                    createGlfwWindow(it.width, it.height, it.title.c_str(), pNewWindow.get());
                if (!result)
                    std::abort();

                pNewWindow->desiredSampleCount = it.msaa;
                pNewWindow->vsync = it.vsync;

                pNewWindow->position = glm::vec3{0.f, 0.f, -17.5f};
                pNewWindow->orientation = glm::quat{1.f, 0.f, 0.f, 0.f};
                pNewWindow->fovY = 60;
                pNewWindow->nearZ = 2;
                pNewWindow->farZ = 50;

#ifdef EDITOR_MODE
                imguiAddGlfwWindow(&windowInfo, pNewWindow.get(), &pNewWindow->keyboard,
                                   &pNewWindow->mouse);
#endif

                glfw_windowData.emplace_back(pNewWindow.release());
            } else if (it.implementation == Settings::Window::Implementation::SDL3) {
                std::unique_ptr<SDL3_WindowData> pNewWindow{new SDL3_WindowData};

                bool result =
                    createSDL3Window(it.width, it.height, it.title.c_str(), pNewWindow.get());

                if (!result)
                    std::abort();

                pNewWindow->desiredSampleCount = it.msaa;
                pNewWindow->vsync = it.vsync;

                pNewWindow->position = glm::vec3{0.f, 0.f, -17.5f};
                pNewWindow->orientation = glm::quat{1.f, 0.f, 0.f, 0.f};
                pNewWindow->fovY = 60;
                pNewWindow->nearZ = 2;
                pNewWindow->farZ = 50;

#ifdef EDITOR_MODE
                imguiAddSDL3Window(&windowInfo, pNewWindow.get(), &pNewWindow->keyboard,
                                   &pNewWindow->mouse);
#endif

                sdl3_windowData.emplace_back(pNewWindow.release());
            }
        }

#ifdef FOE_XR_SUPPORT
        if (settings.xr.enableXr || settings.xr.forceXr) {
            result = createXrRuntime(settings.xr.debugLogging, &xrRuntime);
            if (result.value != FOE_SUCCESS && settings.xr.forceXr) {
                ERRC_END_PROGRAM
            }

            xrData.desiredSampleCount = settings.xr.msaa;
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

            if (!getSDL3VkExtensions(&extensionCount, &ppExtensionNames))
                std::abort();
            for (uint32_t i = 0; i < extensionCount; ++i) {
                vkInstanceExtensions.emplace_back(ppExtensionNames[i]);
            }
        }

        result =
            createGfxRuntime(xrRuntime, settings.graphics.validation,
                             settings.graphics.debugLogging, {}, vkInstanceExtensions, &gfxRuntime);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM
        }

        for (auto it : glfw_windowData) {
            if (createGlfwWindowVkSurface(gfxRuntime, FOE_NULL_HANDLE, it, nullptr,
                                          &it->renderSurfaceData.surface) != FOE_SUCCESS)
                std::abort();
        }

        // sdl3
        for (auto &it : sdl3_windowData) {
            if (createSDL3WindowVkSurface(gfxRuntime, FOE_NULL_HANDLE, it, nullptr,
                                          &it->renderSurfaceData.surface) != FOE_SUCCESS)
                std::abort();
        }

        std::vector<VkSurfaceKHR> surfaces;
        // glfw
        for (auto const it : glfw_windowData) {
            surfaces.push_back(it->renderSurfaceData.surface);
        }
        for (auto const it : sdl3_windowData) {
            surfaces.push_back(it->renderSurfaceData.surface);
        }

        result = createGfxSession(
            gfxRuntime, xrRuntime, settings.general.enableWindows || !glfw_windowData.empty(),
            std::move(surfaces), settings.graphics.gpu, settings.xr.forceXr, &gfxSession);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM
        }

        // msaa - wsi - GLFW
        for (auto &window : glfw_windowData) {
            window->sampleCount = window->renderSurfaceData.sampleCount =
                foeGfxVkGetBestSupportedMSAA(gfxSession, window->desiredSampleCount);
        }

        // msaa - wsi - SDL3
        for (auto &window : sdl3_windowData) {
            window->sampleCount = window->renderSurfaceData.sampleCount =
                foeGfxVkGetBestSupportedMSAA(gfxSession, window->desiredSampleCount);
        }

#ifdef FOE_XR_SUPPORT
        { // msaa - xr
            xrData.sampleCount =
                foeGfxVkGetBestSupportedMSAA(gfxSession, xrData.desiredSampleCount);
        }
#endif
    }

    result = foeGfxCreateUploadContext(gfxSession, &gfxResUploadContext);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM
    }

    result = foeGfxCreateDelayedCaller(gfxSession, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1,
                                       &gfxDelayedDestructor);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM
    }

    result = foeGfxCreateRenderViewPool(gfxSession, 32, &gfxRenderViewPool);
    if (result.value != FOE_SUCCESS) {
        ERRC_END_PROGRAM
    }

    // glfw
    for (auto it : glfw_windowData) {
        result = foeGfxAllocateRenderView(gfxRenderViewPool, &it->renderView);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM
        }
    }

    // sdl3
    for (auto &it : sdl3_windowData) {
        result = foeGfxAllocateRenderView(gfxRenderViewPool, &it->renderView);
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM
        }
    }

    // Create per-frame data
    for (auto &it : frameData) {
        result = it.create(foeGfxVkGetDevice(gfxSession));
        if (result.value != FOE_SUCCESS) {
            ERRC_END_PROGRAM
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
        result = ::startXR(xrRuntime, gfxSession, gfxDelayedDestructor, depthFormat, true, &xrData);

        // If the user specified to force an XR session and couldn't find/create the
        // session, fail out
        if (settings.xr.forceXr && xrData.session == FOE_NULL_HANDLE) {
            FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL,
                    "XR support enabled but no HMD session was detected/started.")
            return 1;
        }
    }
#endif

    return FOE_SUCCESS;
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
    // sdl3
    for (auto it : sdl3_windowData) {
#ifdef EDITOR_MODE
        windowInfo.removeWindow(it);
#endif
        destroySDL3Window(gfxRuntime, gfxSession, it);
        delete it;
    }
    sdl3_windowData.clear();

    // glfw
    for (auto it : glfw_windowData) {
#ifdef EDITOR_MODE
        windowInfo.removeWindow(it->pWindow);
#endif
        destroyGlfwWindow(gfxRuntime, gfxSession, it);
        delete it;
    }
    glfw_windowData.clear();

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

    MagickCoreTerminus();
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

    FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO, "Entering main loop")
    while ((!glfw_windowData.empty() || !sdl3_windowData.empty())
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
                                    false, &pApplication->xrData);
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
        processGlfwEvents(glfw_windowData.size(), glfw_windowData.data());
        processSDL3Events(sdl3_windowData.size(), sdl3_windowData.data());

#ifdef FOE_XR_SUPPORT
        // Process XR Events
        if (xrRuntime != FOE_NULL_HANDLE)
            foeXrProcessEvents(xrRuntime);
#endif

        // Window Processing
        for (auto it = glfw_windowData.begin(); it != glfw_windowData.end();) {
            GLFW_WindowData *window = *it;

#ifdef EDITOR_MODE
            if (pImGuiRenderWindow == nullptr)
                pImGuiRenderWindow = window;
#endif

            // if window is set to close, check if to be destroyed now
            if (window->requestClose) {
#ifdef EDITOR_MODE
                // if this was the imgui window, change it
                if (pImGuiRenderWindow == window)
                    pImGuiRenderWindow = nullptr;
#endif
                std::scoped_lock<std::mutex> lock{window->renderSurfaceData.sync};
                window->renderSurfaceData.active = false;
                if (window->renderSurfaceData.inFlight != 0) {
                    // still items in-flight, not to be removed yet
                    ++it;
                    continue;
                }
#ifdef EDITOR_MODE
                windowInfo.removeWindow(window->pWindow);
#endif
                destroyGlfwWindow(gfxRuntime, gfxSession, window);
                delete window;

                it = glfw_windowData.erase(it);
                continue;
            }

#ifdef EDITOR_MODE
            // Only the first/primary window supports ImGui interaction
            if (window == pImGuiRenderWindow) {
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
            if (window != pImGuiRenderWindow ||
                (!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse()))
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
            if (window == pImGuiRenderWindow) {
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

        // sdl3
        for (auto it = sdl3_windowData.begin(); it < sdl3_windowData.end();) {
            SDL3_WindowData *window = *it;

#ifdef EDITOR_MODE
            if (pImGuiRenderWindow == nullptr)
                pImGuiRenderWindow = window;
#endif

            // if window is set to close, check if to be destroyed now
            if (window->close) {
#ifdef EDITOR_MODE
                // if this was the imgui window, change it
                if (pImGuiRenderWindow == window)
                    pImGuiRenderWindow = nullptr;
#endif
                std::scoped_lock<std::mutex> lock{window->renderSurfaceData.sync};
                window->renderSurfaceData.active = false;
                if (window->renderSurfaceData.inFlight != 0) {
                    // still items in-flight, not to be removed yet
                    ++it;
                    continue;
                }
#ifdef EDITOR_MODE
                windowInfo.removeWindow(window);
#endif
                destroySDL3Window(gfxRuntime, gfxSession, window);
                delete window;

                it = sdl3_windowData.erase(it);
                continue;
            }

#ifdef EDITOR_MODE
            // Only the first/primary window supports ImGui interaction
            if (window == pImGuiRenderWindow) {
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

                imguiRenderer.keyboardInput(window->keyboard.unicodeChar, imguiSDL3KeyConvert,
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
            if (window != pImGuiRenderWindow ||
                (!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse()))
#endif
            {
                processSDL3UserInput(window, timeElapsedInSec);
            }

            // Check if window was resized, and if so request associated swapchains to
            // be rebuilt
            if (window->resized) {
                window->needSwapchainRebuild = true;
            }

#ifdef EDITOR_MODE
            if (window == pImGuiRenderWindow) {
                int width, height;
                getSDL3WindowLogicalSize(window, &width, &height);
                imguiRenderer.resize(width, height);
                float xScale, yScale;
                getSDL3WindowScale(window, &xScale, &yScale);
                imguiRenderer.rescale(xScale, yScale);
            }
#endif

            ++it;
        }

        // iterate through, clear frames that are complete
        for (PerFrameData &frame : frameData) {
            if (!frame.active)
                continue;

            if (vkWaitForFences(foeGfxVkGetDevice(gfxSession), 1, &frame.frameComplete, VK_TRUE,
                                0) == VK_SUCCESS) {
                // run any frame-complete tasks
                while (!frame.onFrameCompleteTasks.empty()) {
                    auto task = frame.onFrameCompleteTasks.front();
                    frame.onFrameCompleteTasks.pop();

                    task.pfnTask(task.pTaskData);
                }

                // Resource Loader Gfx Maintenance
                for (auto &it : pSimulationSet->resourceLoaders) {
                    if (it.pGfxMaintenanceFn) {
                        it.pGfxMaintenanceFn(it.pLoader);
                    }
                }

                // Reset frame data
                vkResetFences(foeGfxVkGetDevice(gfxSession), 1, &frame.frameComplete);
                vkResetCommandPool(foeGfxVkGetDevice(gfxSession), frame.commandPool, 0);

                // Advance and destroy items related to this frame
                foeGfxRunDelayedCalls(gfxDelayedDestructor);

                frame.active = false;
            }
        }

        // Determine if the next frame is available to start rendering to, if we
        // don't have one
        if (frameIndex == UINT32_MAX) {
            uint32_t nextFrameIndex = (lastFrameIndex + 1) % frameData.size();
            if (!frameData[nextFrameIndex].active)
                frameIndex = nextFrameIndex;
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
            // glfw
            for (auto it : glfw_windowData) {
                // If no window here, skip
                if (it->pWindow == FOE_NULL_HANDLE)
                    continue;
                if (it->renderSurfaceData.swapchain != FOE_NULL_HANDLE && !it->needSwapchainRebuild)
                    continue;

                it->needSwapchainRebuild = false;

                int width, height;
                getGlfwWindowPixelSize(it, &width, &height);

                result = rebuildSurfaceSwapchain(&it->renderSurfaceData, gfxSession,
                                                 gfxDelayedDestructor, it->vsync, width, height,
                                                 depthFormat);
                if (result.value != FOE_SUCCESS)
                    ERRC_END_PROGRAM
            }

            // sdl3
            for (auto it : sdl3_windowData) {
                // If no window here, skip
                if (it->pWindow == FOE_NULL_HANDLE)
                    continue;
                if (it->renderSurfaceData.swapchain != FOE_NULL_HANDLE && !it->needSwapchainRebuild)
                    continue;

                it->needSwapchainRebuild = false;

                int width, height;
                getSDL3WindowPixelSize(it, &width, &height);

                result = rebuildSurfaceSwapchain(&it->renderSurfaceData, gfxSession,
                                                 gfxDelayedDestructor, it->vsync, width, height,
                                                 depthFormat);
                if (result.value != FOE_SUCCESS)
                    ERRC_END_PROGRAM
            }

            // Acquire Target Presentation Images
            struct WindowRenderData {
                bool imguiWindow;
                WindowSurfaceData *pSurfaceData;
                foeGfxRenderView renderView;
                VkSampleCountFlags sampleCount;
            };
            std::vector<WindowRenderData> windowRenderList;

            // glfw
            for (auto &it : glfw_windowData) {
                std::scoped_lock<std::mutex> lock{it->renderSurfaceData.sync};
                if (!it->renderSurfaceData.active)
                    // if this surface is not active, don't render it right now
                    continue;

                result = vk_to_foeResult(
                    foeGfxVkAcquireSwapchainImage(gfxSession, it->renderSurfaceData.swapchain,
                                                  &it->renderSurfaceData.acquiredImageData));
                if (result.value == VK_TIMEOUT || result.value == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (result.value == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchain
                    it->needSwapchainRebuild = true;
                } else if (result.value == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it->needSwapchainRebuild = true;
                    it->renderSurfaceData.acquiredImage = true;
                } else if (result.value) {
                    // Catastrophic error
                    ERRC_END_PROGRAM
                } else {
                    // No issues, add it to be rendered
                    it->renderSurfaceData.acquiredImage = true;
                }

                if (it->renderSurfaceData.acquiredImage) {
                    glm::mat4 matrix = glm::perspectiveFov(
                        glm::radians(it->fovY),
                        (float)it->renderSurfaceData.acquiredImageData.extent.width,
                        (float)it->renderSurfaceData.acquiredImageData.extent.height, it->nearZ,
                        it->farZ);
                    matrix *= glm::mat4_cast(it->orientation) *
                              glm::translate(glm::mat4(1.f), it->position);

                    foeGfxUpdateRenderView(it->renderView, sizeof(glm::mat4), &matrix);

                    ++it->renderSurfaceData.inFlight;
                    windowRenderList.emplace_back(WindowRenderData{
#ifdef EDITOR_MODE
                        .imguiWindow = (it == pImGuiRenderWindow),
#else
                        .imguiWindow = false,
#endif
                        .pSurfaceData = &it->renderSurfaceData,
                        .renderView = it->renderView,
                        .sampleCount = it->sampleCount,
                    });
                }
            }

            // sdl
            for (auto it : sdl3_windowData) {
                std::scoped_lock<std::mutex> lock{it->renderSurfaceData.sync};
                if (!it->renderSurfaceData.active)
                    // if this surface is not active, don't render it right now
                    continue;

                result = vk_to_foeResult(
                    foeGfxVkAcquireSwapchainImage(gfxSession, it->renderSurfaceData.swapchain,
                                                  &it->renderSurfaceData.acquiredImageData));
                if (result.value == VK_TIMEOUT || result.value == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (result.value == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchains
                    it->needSwapchainRebuild = true;
                } else if (result.value == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it->needSwapchainRebuild = true;
                    it->renderSurfaceData.acquiredImage = true;
                } else if (result.value) {
                    // Catastrophic error
                    ERRC_END_PROGRAM
                } else {
                    // No issues, add it to be rendered
                    it->renderSurfaceData.acquiredImage = true;
                }

                if (it->renderSurfaceData.acquiredImage) {
                    glm::mat4 matrix = glm::perspectiveFov(
                        glm::radians(it->fovY),
                        (float)it->renderSurfaceData.acquiredImageData.extent.width,
                        (float)it->renderSurfaceData.acquiredImageData.extent.height, it->nearZ,
                        it->farZ);
                    matrix *= glm::mat4_cast(it->orientation) *
                              glm::translate(glm::mat4(1.f), it->position);

                    foeGfxUpdateRenderView(it->renderView, sizeof(glm::mat4), &matrix);

                    ++it->renderSurfaceData.inFlight;
                    windowRenderList.emplace_back(WindowRenderData{
#ifdef EDITOR_MODE
                        .imguiWindow = (it == pImGuiRenderWindow),
#else
                        .imguiWindow = false,
#endif
                        .pSurfaceData = &it->renderSurfaceData,
                        .renderView = it->renderView,
                        .sampleCount = it->sampleCount,
                    });
                }
            }

            if (windowRenderList.empty()) {
                goto SKIP_FRAME_RENDER;
            }

            frameData[frameIndex].active = true;

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
                            xrData.sampleCount, pSimulationSet, viewDescriptorSet, frameIndex,
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
                                FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_ERROR,
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
                                FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL,
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

            // window surface rendering
            for (auto const &window : windowRenderList) {
                auto *pSurfaceData = window.pSurfaceData;

                result = foeGfxAcquireNextRenderTarget(pSurfaceData->gfxOffscreenRenderTarget,
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
                    foeGfxVkGetRenderTargetImage(pSurfaceData->gfxOffscreenRenderTarget, 0),
                    foeGfxVkGetRenderTargetImageView(pSurfaceData->gfxOffscreenRenderTarget, 0),
                    pSurfaceData->surfaceFormat.format, pSurfaceData->acquiredImageData.extent,
                    VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource,
                    &renderTargetColourImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                result = foeGfxVkImportImageRenderJob(
                    renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                    "renderTargetDepthImage",
                    foeGfxVkGetRenderTargetImage(pSurfaceData->gfxOffscreenRenderTarget, 1),
                    foeGfxVkGetRenderTargetImageView(pSurfaceData->gfxOffscreenRenderTarget, 1),
                    depthFormat, pSurfaceData->acquiredImageData.extent, VK_IMAGE_LAYOUT_UNDEFINED,
                    true, {}, &renderTargetDepthImageResource, &renderTargetDepthImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                VkDescriptorSet cameraProjViewDescriptor =
                    foeGfxVkGetRenderViewDescriptorSet(gfxRenderViewPool, window.renderView);

                foeGfxVkRenderGraphJob renderSceneJobHandle;
                result = renderSceneJob(
                    renderGraph, "render3dScene", VK_NULL_HANDLE, renderTargetColourImageResource,
                    1, &renderTargetColourImportJob, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    renderTargetDepthImageResource, 1, &renderTargetDepthImportJob,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, window.sampleCount, pSimulationSet,
                    cameraProjViewDescriptor, frameIndex, &renderSceneJobHandle);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                foeGfxVkRenderGraphResource presentImageResource;
                foeGfxVkRenderGraphJob presentImageImportJob;

                result = foeGfxVkImportSwapchainImageRenderJob(
                    renderGraph, "importPresentationImage", VK_NULL_HANDLE, "presentImage",
                    pSurfaceData->acquiredImageData.swapchain,
                    pSurfaceData->acquiredImageData.imageIndex,
                    pSurfaceData->acquiredImageData.image, pSurfaceData->acquiredImageData.view,
                    pSurfaceData->surfaceFormat.format, pSurfaceData->acquiredImageData.extent,
                    VK_IMAGE_LAYOUT_UNDEFINED, pSurfaceData->acquiredImageData.readySemaphore,
                    &presentImageResource, &presentImageImportJob);
                if (result.value != FOE_SUCCESS) {
                    ERRC_END_PROGRAM
                }

                foeGfxVkRenderGraphJob resolveOrCopyJob;
                if (foeGfxVkGetRenderTargetSamples(pSurfaceData->gfxOffscreenRenderTarget) !=
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

                static CpuImageData *pCpuImageData = nullptr;

                if (frame == 100 && pCpuImageData == nullptr) {
                    auto extent = pSurfaceData->acquiredImageData.extent;
                    pCpuImageData = addCpuImageExport(
                        gfxSession, renderGraph, extent, "test.png",
                        renderTargetColourImageResource,
                        foeGfxVkGetRenderTargetSamples(pSurfaceData->gfxOffscreenRenderTarget),
                        renderSceneJobHandle);
                }

                if (pCpuImageData && pCpuImageData->fence != VK_NULL_HANDLE) {
                    VkResult vkRes =
                        vkGetFenceStatus(foeGfxVkGetDevice(gfxSession), pCpuImageData->fence);
                    if (vkRes == VK_SUCCESS) {
                        foeScheduleAsyncTask(threadPool, renderedImageToFile, pCpuImageData);
                        pCpuImageData = nullptr;
                    }
                }

                foeGfxVkRenderGraphJob renderDebugUiJob = FOE_NULL_HANDLE;
#ifdef EDITOR_MODE
                // ImGui only renders on the first/primary window
                if (window.imguiWindow) {
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

                pSurfaceData->acquiredImage = false;

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

                // set task to run when the frameComplete fence clears
                auto onFrameCompleteTask = [](void *pTaskData) {
                    WindowSurfaceData *pSurfaceData = static_cast<WindowSurfaceData *>(pTaskData);

                    pSurfaceData->sync.lock();
                    --pSurfaceData->inFlight;
                    pSurfaceData->sync.unlock();
                };

                frameData[frameIndex].onFrameCompleteTasks.emplace(OnFrameCompleteTask{
                    .pfnTask = onFrameCompleteTask,
                    .pTaskData = pSurfaceData,
                });
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
    FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO, "Exiting main loop")

    return 0;
}