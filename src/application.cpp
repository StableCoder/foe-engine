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

#include "application.hpp"

#include <GLFW/glfw3.h>
#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/ecs/editor_name_map.hpp>
#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/graphics/vk/mesh.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/graphics/vk/queue_family.hpp>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/job/blit_image.hpp>
#include <foe/graphics/vk/render_graph/job/export_image.hpp>
#include <foe/graphics/vk/render_graph/job/import_image.hpp>
#include <foe/graphics/vk/render_graph/job/present_image.hpp>
#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>
#include <foe/graphics/vk/render_pass_pool.hpp>
#include <foe/graphics/vk/render_target.hpp>
#include <foe/graphics/vk/runtime.hpp>
#include <foe/graphics/vk/sample_count.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/graphics/vk/shader.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/system.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/quaternion_math.hpp>
#include <foe/resource/armature.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/search_paths.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/wsi/keyboard.hpp>
#include <foe/wsi/mouse.hpp>
#include <foe/wsi/vulkan.h>
#include <vk_error_code.hpp>

#include "armature_system.hpp"
#include "camera_pool.hpp"
#include "camera_system.hpp"
#include "graphics.hpp"
#include "log.hpp"
#include "logging.hpp"
#include "position_descriptor_pool.hpp"
#include "register_basic_functionality.hpp"
#include "render_state_pool.hpp"
#include "vk_animation.hpp"

#include "render_graph/render_scene.hpp"

#ifdef FOE_XR_SUPPORT
#include <foe/xr/openxr/core.hpp>
#include <foe/xr/openxr/error_code.hpp>
#include <foe/xr/openxr/runtime.hpp>
#include <foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp>

#include "xr.hpp"
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
        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,         \
                errC.message());                                                                   \
        return std::make_tuple(false, errC.value());                                               \
    }

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,         \
                errC.message());                                                                   \
        return errC.value();                                                                       \
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

namespace {

template <typename ResourcePool>
auto getResourcePool(foeResourcePoolBase **pResourcePools, size_t poolCount) -> ResourcePool * {
    ResourcePool *pPool{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pPool = dynamic_cast<ResourcePool *>(pResourcePools[i]);
        if (pPool != nullptr)
            break;
    }

    return pPool;
}

template <typename ResourceLoader>
auto getResourceLoader(foeSimulationLoaderData *pResourceLoaders, size_t poolCount)
    -> ResourceLoader * {
    ResourceLoader *pLoader{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pLoader = dynamic_cast<ResourceLoader *>(pResourceLoaders[i].pLoader);
        if (pLoader != nullptr)
            break;
    }

    return pLoader;
}

template <typename ComponentPool>
auto getComponentPool(foeComponentPoolBase **pComponentPools, size_t poolCount) -> ComponentPool * {
    ComponentPool *pPool{nullptr};
    for (size_t i = 0; i < poolCount; ++i) {
        pPool = dynamic_cast<ComponentPool *>(pComponentPools[i]);
        if (pPool != nullptr)
            break;
    }

    return pPool;
}

template <typename System>
auto getSystem(foeSystemBase **pSystems, size_t systemCount) -> System * {
    System *pSystem{nullptr};
    for (size_t i = 0; i < systemCount; ++i) {
        pSystem = dynamic_cast<System *>(pSystems[i]);
        if (pSystem != nullptr)
            break;
    }

    return pSystem;
}

auto renderCall(foeId entity,
                foeRenderState const *pRenderState,
                foeGfxSession gfxSession,
                foeSimulationState *pSimulationSet,
                VkCommandBuffer commandBuffer,
                VkSampleCountFlags samples,
                VkRenderPass renderPass,
                VkDescriptorSet cameraDescriptor) -> bool {
    VkDescriptorSet const dummyDescriptorSet = foeGfxVkGetDummySet(gfxSession);

    foeVertexDescriptor *pVertexDescriptor{nullptr};
    bool boned{false};
    if (pRenderState->bonedVertexDescriptor != FOE_INVALID_ID &&
        pRenderState->boneDescriptorSet != VK_NULL_HANDLE) {
        boned = true;
        pVertexDescriptor =
            getResourcePool<foeVertexDescriptorPool>(pSimulationSet->resourcePools.data(),
                                                     pSimulationSet->resourcePools.size())
                ->find(pRenderState->bonedVertexDescriptor);
    }

    if (pVertexDescriptor == nullptr) {
        pVertexDescriptor =
            getResourcePool<foeVertexDescriptorPool>(pSimulationSet->resourcePools.data(),
                                                     pSimulationSet->resourcePools.size())
                ->find(pRenderState->vertexDescriptor);
    }

    auto *pMaterial = getResourcePool<foeMaterialPool>(pSimulationSet->resourcePools.data(),
                                                       pSimulationSet->resourcePools.size())
                          ->find(pRenderState->material);
    auto *pMesh = getResourcePool<foeMeshPool>(pSimulationSet->resourcePools.data(),
                                               pSimulationSet->resourcePools.size())
                      ->find(pRenderState->mesh);

    if (pVertexDescriptor == nullptr || pMaterial == nullptr || pMesh == nullptr) {
        return false;
    }
    if (pVertexDescriptor->getState() != foeResourceState::Loaded ||
        pMaterial->getState() != foeResourceState::Loaded ||
        pMesh->getState() != foeResourceState::Loaded) {
        return false;
    }

    // Retrieve the pipeline
    auto *pGfxVertexDescriptor = &pVertexDescriptor->data.vertexDescriptor;
    VkPipelineLayout layout;
    uint32_t descriptorSetLayoutCount;
    VkPipeline pipeline;

    foeGfxVkGetPipelinePool(gfxSession)
        ->getPipeline(const_cast<foeGfxVertexDescriptor *>(pGfxVertexDescriptor),
                      pMaterial->data.pGfxFragDescriptor, renderPass, 0, samples, &layout,
                      &descriptorSetLayoutCount, &pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            &cameraDescriptor, 0, nullptr);

    foeGfxVkBindMesh(pMesh->data.gfxData, commandBuffer, boned);

    auto vertSetLayouts = pGfxVertexDescriptor->getBuiltinSetLayouts();
    if (vertSetLayouts & FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX) {
        auto *pPosition3dPool = getComponentPool<foePosition3dPool>(
            pSimulationSet->componentPools.data(), pSimulationSet->componentPools.size());

        auto posOffset = pPosition3dPool->find(entity);

        // If can't find a position, return failure
        if (posOffset == pPosition3dPool->size())
            return false;

        auto *pPosition = (pPosition3dPool->begin<1>() + posOffset)->get();
        // Bind the object's position *if* the descriptor supports it
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &pPosition->descriptorSet, 0, nullptr);
    } else {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                                &dummyDescriptorSet, 0, nullptr);
    }
    if (boned) {
        // If we have bone information, bind that too
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
                                &pRenderState->boneDescriptorSet, 0, nullptr);
    }
    // Bind the fragment descriptor set *if* it exists?
    if (auto set = pMaterial->data.materialDescriptorSet; set != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                foeDescriptorSetLayoutIndex::FragmentShader, 1, &set, 0, nullptr);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDrawIndexed(commandBuffer, foeGfxGetMeshIndices(pMesh->data.gfxData), 1, 0, 0, 0);

    return true;
}

} // namespace

#include "state_import/import_state.hpp"

auto Application::initialize(int argc, char **argv) -> std::tuple<bool, int> {
    std::error_code errC;

    initializeLogging();
    errC = registerBasicFunctionality();
    if (errC) {
        FOE_LOG(General, Fatal, "Error registering basic functionality with error: {} - {}",
                errC.value(), errC.message())
        return std::make_tuple(false, errC.value());
    }

    foeSearchPaths searchPaths;

    auto [continueRun, retVal] = loadSettings(argc, argv, settings, searchPaths);
    if (!continueRun) {
        return std::make_tuple(false, retVal);
    }

    errC = foeCreateThreadPool(1, 1, &threadPool);
    if (errC)
        ERRC_END_PROGRAM_TUPLE

    errC = foeStartThreadPool(threadPool);
    if (errC)
        ERRC_END_PROGRAM_TUPLE

    auto asyncTaskFunc = [&](std::function<void()> task) {
        task();
        // foeScheduleAsyncTask(threadPool, std::move(task));
    };

    errC = importState("persistent", &searchPaths, asyncTaskFunc, &pSimulationSet);
    if (errC) {
        FOE_LOG(General, Fatal, "Error importing '{}' state with error: {} - {}", "persistent",
                errC.value(), errC.message())
        return std::make_tuple(false, errC.value());
    }

    // Special Entities
    cameraID = pSimulationSet->pEntityNameMap->find("camera");

#ifdef EDITOR_MODE
    auto *pImGuiContext = ImGui::CreateContext();

    errC = registerImGui(&imguiRegistrar);
    if (errC)
        ERRC_END_PROGRAM_TUPLE

    foeLogger::instance()->registerSink(&devConsole);

    imguiState.setImGuiContext(pImGuiContext);
    imguiRenderer.setImGuiContext(pImGuiContext);

    uiSave.registerUI(&imguiState);
    devConsole.registerUI(&imguiState);
    fileTermination.registerUI(&imguiState);
    viewFrameTimeInfo.registerUI(&imguiState);
    windowInfo.registerUI(&imguiState);

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
            errC = foeWsiCreateWindow(settings.window.width, settings.window.height, "FoE Engine",
                                      true, &it.window);
            if (errC)
                ERRC_END_PROGRAM_TUPLE

#ifdef EDITOR_MODE
            windowInfo.addWindow(it.window);
#endif
        }

#ifdef FOE_XR_SUPPORT
        if (settings.xr.enableXr || settings.xr.forceXr) {
            errC = createXrRuntime(settings.xr.debugLogging, &xrRuntime);
            if (errC && settings.xr.forceXr) {
                ERRC_END_PROGRAM_TUPLE
            }
        }
#endif

        errC = createGfxRuntime(xrRuntime, settings.window.enableWSI, settings.graphics.validation,
                                settings.graphics.debugLogging, &gfxRuntime);
        if (errC) {
            ERRC_END_PROGRAM_TUPLE
        }

        for (auto &it : windowData) {
            errC =
                foeWsiWindowGetVkSurface(it.window, foeGfxVkGetInstance(gfxRuntime), &it.surface);
            if (errC)
                ERRC_END_PROGRAM_TUPLE
        }

        std::vector<VkSurfaceKHR> surfaces;
        for (auto const &it : windowData) {
            surfaces.push_back(it.surface);
        }

        errC =
            createGfxSession(gfxRuntime, xrRuntime, settings.window.enableWSI, std::move(surfaces),
                             settings.graphics.gpu, settings.xr.forceXr, &gfxSession);
        if (errC) {
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

    errC = foeGfxCreateUploadContext(gfxSession, &gfxResUploadContext);
    if (errC) {
        ERRC_END_PROGRAM_TUPLE
    }

    errC = foeGfxCreateDelayedDestructor(gfxSession, FOE_GRAPHICS_MAX_BUFFERED_FRAMES,
                                         &gfxDelayedDestructor);
    if (errC) {
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
        errC = it.create(foeGfxVkGetDevice(gfxSession));
        if (errC) {
            ERRC_END_PROGRAM_TUPLE
        }
    }

    { // Initialize simulation
        foeSimulationInitInfo simInitInfo{
            .gfxSession = gfxSession,
            .externalFileSearchFn = std::bind(&foeGroupData::findExternalFile,
                                              &pSimulationSet->groupData, std::placeholders::_1),
            .asyncJobFn = asyncTaskFunc,
        };
        foeInitializeSimulation(pSimulationSet, &simInitInfo);
    }

    { // Load all available resources
        for (auto *ptr : getResourcePool<foeArmaturePool>(pSimulationSet->resourcePools.data(),
                                                          pSimulationSet->resourcePools.size())
                             ->getDataVector()) {
            ptr->loadResource(false);
        }

        for (auto *ptr : getResourcePool<foeMaterialPool>(pSimulationSet->resourcePools.data(),
                                                          pSimulationSet->resourcePools.size())
                             ->getDataVector()) {
            ptr->loadResource(false);
        }

        for (auto *ptr : getResourcePool<foeMeshPool>(pSimulationSet->resourcePools.data(),
                                                      pSimulationSet->resourcePools.size())
                             ->getDataVector()) {
            ptr->loadResource(false);
        }

        for (auto *ptr : getResourcePool<foeShaderPool>(pSimulationSet->resourcePools.data(),
                                                        pSimulationSet->resourcePools.size())
                             ->getDataVector()) {
            ptr->loadResource(false);
        }

        for (auto *ptr :
             getResourcePool<foeVertexDescriptorPool>(pSimulationSet->resourcePools.data(),
                                                      pSimulationSet->resourcePools.size())
                 ->getDataVector()) {
            ptr->loadResource(false);
        }
    }

#ifdef FOE_XR_SUPPORT
    if (settings.xr.enableXr || settings.xr.forceXr) {
        startXR(true);

        // If the user specified to force an XR session and couldn't find/create the session, fail
        // out
        if (settings.xr.forceXr && xrSession.session == XR_NULL_HANDLE) {
            FOE_LOG(General, Fatal, "XR support enabled but no HMD session was detected/started.")
            return std::make_tuple(false, 1);
        }
    }
#endif

    return std::make_tuple(true, 0);
}

void Application::deinitialize() {
    std::error_code errC;

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxWaitIdle(gfxSession);

    // Systems Deinitialization
    getSystem<foePhysicsSystem>(pSimulationSet->systems.data(), pSimulationSet->systems.size())
        ->deinitialize();

    getSystem<foeArmatureSystem>(pSimulationSet->systems.data(), pSimulationSet->systems.size())
        ->deinitialize();

    { // Resource Unloading
        auto *pCollisionShapePool = getResourcePool<foeCollisionShapePool>(
            pSimulationSet->resourcePools.data(), pSimulationSet->resourcePools.size());
        pCollisionShapePool->unloadAll();

        auto *pCollisionShapeLoader = getResourceLoader<foeCollisionShapeLoader>(
            pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
        pCollisionShapeLoader->maintenance();

        auto *pArmaturePool = getResourcePool<foeArmaturePool>(
            pSimulationSet->resourcePools.data(), pSimulationSet->resourcePools.size());
        pArmaturePool->unloadAll();

        auto *pMeshPool = getResourcePool<foeMeshPool>(pSimulationSet->resourcePools.data(),
                                                       pSimulationSet->resourcePools.size());
        pMeshPool->unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            auto *pMeshLoader = getResourceLoader<foeMeshLoader>(
                pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
            pMeshLoader->gfxMaintenance();
        }

        auto *pMaterialPool = getResourcePool<foeMaterialPool>(
            pSimulationSet->resourcePools.data(), pSimulationSet->resourcePools.size());
        pMaterialPool->unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            auto *pMaterialLoader = getResourceLoader<foeMaterialLoader>(
                pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
            pMaterialLoader->gfxMaintenance();
        }

        auto *pImagePool = getResourcePool<foeImagePool>(pSimulationSet->resourcePools.data(),
                                                         pSimulationSet->resourcePools.size());
        pImagePool->unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            auto *pImageLoader = getResourceLoader<foeImageLoader>(
                pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
            pImageLoader->gfxMaintenance();
        }

        auto *pVertexDescriptorPool = getResourcePool<foeVertexDescriptorPool>(
            pSimulationSet->resourcePools.data(), pSimulationSet->resourcePools.size());
        pVertexDescriptorPool->unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            auto *pVertexDescriptorLoader = getResourceLoader<foeVertexDescriptorLoader>(
                pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
            pVertexDescriptorLoader->gfxMaintenance();
        }

        auto *pShaderPool = getResourcePool<foeShaderPool>(pSimulationSet->resourcePools.data(),
                                                           pSimulationSet->resourcePools.size());
        pShaderPool->unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            auto *pShaderLoader = getResourceLoader<foeShaderLoader>(
                pSimulationSet->resourceLoaders.data(), pSimulationSet->resourceLoaders.size());
            pShaderLoader->gfxMaintenance();
        }
    }

    // Deinit simulation
    if (pSimulationSet)
        foeDeinitializeSimulation(pSimulationSet);

#ifdef FOE_XR_SUPPORT
    stopXR(true);

    if (xrRuntime != FOE_NULL_HANDLE)
        errC = foeXrDestroyRuntime(xrRuntime);
#endif

    // Cleanup per-frame data
    for (auto &it : frameData)
        it.destroy(foeGfxVkGetDevice(gfxSession));
    for (auto &it : swapImageFramebuffers)
        vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);

#ifdef EDITOR_MODE
    imguiRenderer.deinitialize(gfxSession);
#endif

    // Destroy window data
    for (auto &it : windowData) {
#ifdef EDITOR_MODE
        windowInfo.removeWindow(it.window);
#endif
        if (it.gfxOffscreenRenderTarget != FOE_NULL_HANDLE)
            foeGfxDestroyRenderTarget(it.gfxOffscreenRenderTarget);
        it.gfxOffscreenRenderTarget = FOE_NULL_HANDLE;

        it.swapchain.destroy(foeGfxVkGetDevice(gfxSession));

        if (it.surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(foeGfxVkGetInstance(gfxRuntime), it.surface, nullptr);
        it.surface = VK_NULL_HANDLE;

        if (it.window != FOE_NULL_HANDLE)
            foeWsiDestroyWindow(it.window);
        it.window = FOE_NULL_HANDLE;
    }

    // Cleanup graphics
    if (gfxDelayedDestructor != FOE_NULL_HANDLE)
        foeGfxDestroyDelayedDestructor(gfxDelayedDestructor);
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

std::error_code Application::startXR(bool localPoll) {
    std::error_code errC;

    if (xrRuntime == FOE_NULL_HANDLE) {
        FOE_LOG(General, Error, "Tried to start an XR session, but no XR runtime has been started");
    }
#ifdef FOE_XR_SUPPORT
    else {
        errC = createXrSession(xrRuntime, gfxSession, &xrSession);
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
            goto START_XR_FAILED;
        }

        // OpenXR Session Begin

        // Wait for the session to be ready
        while (xrSession.state != XR_SESSION_STATE_READY) {
            if (localPoll) {
                errC = foeXrProcessEvents(xrRuntime);
                if (errC) {
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, errC.message());
                    goto START_XR_FAILED;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        // Session Views
        uint32_t viewConfigViewCount;
        errC =
            xrEnumerateViewConfigurationViews(foeOpenXrGetInstance(xrRuntime), xrSession.systemId,
                                              xrSession.type, 0, &viewConfigViewCount, nullptr);
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
            goto START_XR_FAILED;
        }

        std::vector<XrViewConfigurationView> viewConfigs;
        viewConfigs.resize(viewConfigViewCount);

        errC = xrEnumerateViewConfigurationViews(
            foeOpenXrGetInstance(xrRuntime), xrSession.systemId, xrSession.type, viewConfigs.size(),
            &viewConfigViewCount, viewConfigs.data());
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
            goto START_XR_FAILED;
        }
        xrViews.clear();
        for (auto const &it : viewConfigs) {
            xrViews.emplace_back(foeXrVkSessionView{.viewConfig = it});
        }

        // OpenXR Swapchains
        std::vector<int64_t> swapchainFormats;
        errC = foeOpenXrEnumerateSwapchainFormats(xrSession.session, swapchainFormats);
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
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
            errC =
                foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedDestructor, offscreenSpecs.data(),
                                           offscreenSpecs.size(), globalMSAA, &newRenderTarget);
            if (errC) {
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        errC.message());
                goto START_XR_FAILED;
            }

            foeGfxUpdateRenderTargetExtent(newRenderTarget,
                                           view.viewConfig.recommendedImageRectWidth,
                                           view.viewConfig.recommendedImageRectHeight);

            errC = foeGfxAcquireNextRenderTarget(newRenderTarget, FOE_GRAPHICS_MAX_BUFFERED_FRAMES);

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

            errC = xrCreateSwapchain(xrSession.session, &swapchainCI, &view.swapchain);
            if (errC) {
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        errC.message());
                goto START_XR_FAILED;
            }

            // Images
            errC = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, view.images);
            if (errC) {
                FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                        errC.message());
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
                errC = vkCreateImageView(foeGfxVkGetDevice(gfxSession), &viewCI, nullptr, &vkView);
                if (errC) {
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, errC.message());
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
                errC = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr,
                                           &newFramebuffer);
                if (errC) {
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, errC.message());
                    goto START_XR_FAILED;
                }

                view.framebuffers.emplace_back(newFramebuffer);
            }
        }

        errC = xrVkCameraSystem.initialize(gfxSession);
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
            goto START_XR_FAILED;
        }

        errC = xrSession.beginSession();
        if (errC) {
            FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,
                    errC.message());
            goto START_XR_FAILED;
        }

        FOE_LOG(General, Info, "Started new XR session {}", static_cast<void *>(xrSession.session));
    }

START_XR_FAILED:
    if (errC) {
        stopXR(localPoll);
    }
#endif // FOE_XR_SUPPORT

    return errC;
}

std::error_code Application::stopXR(bool localPoll) {
    std::error_code errC;

#ifdef FOE_XR_SUPPORT
    if (xrSession.session != XR_NULL_HANDLE) {
        xrSession.requestExitSession();

        while (xrSession.state != XR_SESSION_STATE_STOPPING) {
            if (localPoll) {
                errC = foeXrProcessEvents(xrRuntime);
                {
                    if (errC)
                        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                                __LINE__, errC.message());
                    return errC;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        xrSession.endSession();

        while (xrSession.state != XR_SESSION_STATE_IDLE) {
            if (localPoll) {
                errC = foeXrProcessEvents(xrRuntime);
                if (errC) {
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, errC.message());
                    return errC;
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

        while (xrSession.state != XR_SESSION_STATE_EXITING) {
            if (localPoll) {
                errC = foeXrProcessEvents(xrRuntime);
                if (errC) {
                    FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__,
                            __LINE__, errC.message());
                    return errC;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        xrSession.destroySession();
    }

#endif // FOE_XR_SUPPORT

    return errC;
}

int Application::mainloop() {
    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock{std::chrono::nanoseconds{0}};

    std::error_code errC;

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

#ifdef FOE_XR_SUPPORT
        /*
        static auto nextFireTime =
            programClock.currentTime<std::chrono::seconds>() + std::chrono::seconds(10);
        if (programClock.currentTime<std::chrono::seconds>() > nextFireTime) {
            nextFireTime =
                programClock.currentTime<std::chrono::seconds>() + std::chrono::seconds(10);

            if (xrSession.session == XR_NULL_HANDLE) {
                foeScheduleAsyncTask(threadPool, [&]() { startXR(false); });
            } else {
                foeScheduleAsyncTask(threadPool, [&]() { stopXR(false); });
            }
        }
        */
#endif

        // Component Pool Maintenance
        for (auto &it : pSimulationSet->componentPools) {
            it->maintenance();
        }

        // Resource Loader Maintenance
        for (auto &it : pSimulationSet->resourceLoaders) {
            if (it.pMaintenanceFn) {
                it.pMaintenanceFn(it.pLoader);
            }
        }

        // Process systems
        getSystem<foeArmatureSystem>(pSimulationSet->systems.data(), pSimulationSet->systems.size())
            ->process(timeElapsedInSec);
        getSystem<foePhysicsSystem>(pSimulationSet->systems.data(), pSimulationSet->systems.size())
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
            auto pPosition3dPool = getComponentPool<foePosition3dPool>(
                pSimulationSet->componentPools.data(), pSimulationSet->componentPools.size());
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
                foeGfxRunDelayedDestructor(gfxDelayedDestructor);
            }
        }

        // If we have a frame we can render to, proceed to check for ready-to-render data
        if (frameIndex != UINT32_MAX) {
#ifdef FOE_XR_SUPPORT
            // Lock rendering to OpenXR framerate, which overrides regular rendering
            bool xrAcquiredFrame = false;
            if (xrSession.session != XR_NULL_HANDLE && xrSession.active) {
                xrAcquiredFrame = true;

                XrFrameWaitInfo frameWaitInfo{.type = XR_TYPE_FRAME_WAIT_INFO};
                xrFrameState = XrFrameState{.type = XR_TYPE_FRAME_STATE};
                errC = xrWaitFrame(xrSession.session, &frameWaitInfo, &xrFrameState);
                if (errC) {
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
                auto pCameraPool = getComponentPool<foeCameraPool>(
                    pSimulationSet->componentPools.data(), pSimulationSet->componentPools.size());

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
                errC = it.swapchain.acquireNextImage(foeGfxVkGetDevice(gfxSession));
                if (errC == VK_TIMEOUT || errC == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (errC == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchains
                    it.swapchain.needRebuild();
                } else if (errC == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it.swapchain.needRebuild();
                    windowRenderList.emplace_back(&it);
                } else if (errC) {
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
            getSystem<foeCameraSystem>(pSimulationSet->systems.data(),
                                       pSimulationSet->systems.size())
                ->processCameras(frameIndex);

            getSystem<PositionDescriptorPool>(pSimulationSet->systems.data(),
                                              pSimulationSet->systems.size())
                ->generatePositionDescriptors(frameIndex);

            getSystem<VkAnimationPool>(pSimulationSet->systems.data(),
                                       pSimulationSet->systems.size())
                ->uploadBoneOffsets(frameIndex);

#ifdef FOE_XR_SUPPORT
            // OpenXR Render Section
            if (xrAcquiredFrame) {
                XrFrameBeginInfo frameBeginInfo{.type = XR_TYPE_FRAME_BEGIN_INFO};
                errC = xrBeginFrame(xrSession.session, &frameBeginInfo);
                if (errC) {
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
                        .displayTime = xrFrameState.predictedDisplayTime,
                        .space = xrSession.space,
                    };
                    XrViewState viewState{.type = XR_TYPE_VIEW_STATE};
                    std::vector<XrView> views{xrViews.size(), XrView{.type = XR_TYPE_VIEW}};
                    uint32_t viewCountOutput;
                    errC = xrLocateViews(xrSession.session, &viewLocateInfo, &viewState,
                                         views.size(), &viewCountOutput, views.data());
                    if (errC) {
                        ERRC_END_PROGRAM
                    }

                    for (size_t i = 0; i < views.size(); ++i) {
                        projectionViews[i].pose = views[i].pose;
                        projectionViews[i].fov = views[i].fov;

                        xrViews[i].camera.nearZ = 1;
                        xrViews[i].camera.farZ = 100;

                        xrViews[i].camera.fov = views[i].fov;
                        xrViews[i].camera.pose = views[i].pose;

                        auto pPosition3dPool = getComponentPool<foePosition3dPool>(
                            pSimulationSet->componentPools.data(),
                            pSimulationSet->componentPools.size());
                        auto *pCameraPosition =
                            (pPosition3dPool->begin<1>() + pPosition3dPool->find(cameraID));
                        xrViews[i].camera.pPosition3D = pCameraPosition->get();
                    }

                    xrVkCameraSystem.processCameras(frameIndex, xrViews);

                    // Render Code
                    for (size_t i = 0; i < xrViews.size(); ++i) {
                        auto &it = xrViews[i];
                        auto &renderTarget = xrOffscreenRenderTargets[i];

                        errC = foeGfxAcquireNextRenderTarget(renderTarget,
                                                             FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
                        if (errC)
                            ERRC_END_PROGRAM

                        XrSwapchainImageAcquireInfo acquireInfo{
                            .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};

                        uint32_t newIndex;
                        errC = xrAcquireSwapchainImage(it.swapchain, &acquireInfo, &newIndex);
                        if (errC) {
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
                        errC = foeGfxVkCreateRenderGraph(&renderGraph);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphResource renderTargetColourImageResource;
                        foeGfxVkRenderGraphResource renderTargetDepthImageResource;

                        errC = foeGfxVkImportImageRenderJob(
                            renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                            foeGfxVkGetRenderTargetImage(renderTarget, 0),
                            foeGfxVkGetRenderTargetImageView(renderTarget, 0), it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        errC = foeGfxVkImportImageRenderJob(
                            renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                            "renderTargetDepthImage", foeGfxVkGetRenderTargetImage(renderTarget, 1),
                            foeGfxVkGetRenderTargetImageView(renderTarget, 1), depthFormat,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetDepthImageResource);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkRenderGraphResource xrSwapchainImageResource;
                        errC = foeOpenXrVkImportSwapchainImageRenderJob(
                            renderGraph, "importXrViewSwapchainImage", VK_NULL_HANDLE,
                            "importXrViewSwapchainImage", it.swapchain, it.images[newIndex].image,
                            it.imageViews[newIndex], it.format,
                            VkExtent2D{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            },
                            &xrSwapchainImageResource);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        RenderSceneOutputResources output;
                        errC = renderSceneJob(
                            renderGraph, "render3dScene", VK_NULL_HANDLE,
                            renderTargetColourImageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderTargetDepthImageResource,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            globalMSAA, pSimulationSet, it.camera.descriptor, output);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        renderTargetColourImageResource = output.colourRenderTarget;
                        renderTargetDepthImageResource = output.depthRenderTarget;

                        if (foeGfxVkGetRenderTargetSamples(renderTarget) != VK_SAMPLE_COUNT_1_BIT) {
                            // Resolve
                            ResolveJobUsedResources resources;
                            errC = foeGfxVkResolveImageRenderJob(
                                renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &resources);
                            if (errC) {
                                ERRC_END_PROGRAM
                            }

                            renderTargetColourImageResource = resources.srcImage;
                            xrSwapchainImageResource = resources.dstImage;
                        } else {
                            // Copy
                            BlitJobUsedResources resources;
                            errC = foeGfxVkBlitImageRenderJob(
                                renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                                renderTargetColourImageResource,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, xrSwapchainImageResource,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &resources);
                            if (errC) {
                                ERRC_END_PROGRAM
                            }

                            renderTargetColourImageResource = resources.srcImage;
                            xrSwapchainImageResource = resources.dstImage;
                        }

                        errC = foeGfxVkExportImageRenderJob(
                            renderGraph, "exportPresentationImage", VK_NULL_HANDLE,
                            xrSwapchainImageResource, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {});
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        errC = foeGfxVkExecuteRenderGraph(renderGraph, gfxSession,
                                                          gfxDelayedDestructor);
                        if (errC) {
                            ERRC_END_PROGRAM
                        }

                        foeGfxVkExecuteRenderGraphCpuJobs(renderGraph);

                        foeGfxVkDestroyRenderGraph(renderGraph);
                    }

                    // Assemble composition layers
                    layerProj = XrCompositionLayerProjection{
                        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
                        .layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT,
                        .space = xrSession.space,
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
                errC = xrEndFrame(xrSession.session, &endFrameInfo);
                if (errC) {
                    ERRC_END_PROGRAM
                }
            }
#endif

            auto &window = windowRenderList[0];

            errC = foeGfxAcquireNextRenderTarget(window->gfxOffscreenRenderTarget,
                                                 FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
            if (errC) {
                ERRC_END_PROGRAM
            }

            foeGfxVkRenderGraph renderGraph;
            errC = foeGfxVkCreateRenderGraph(&renderGraph);
            if (errC) {
                ERRC_END_PROGRAM
            }

            foeGfxVkRenderGraphResource renderTargetColourImageResource;
            foeGfxVkRenderGraphResource renderTargetDepthImageResource;

            errC = foeGfxVkImportImageRenderJob(
                renderGraph, "importRenderedImage", VK_NULL_HANDLE, "renderedImage",
                foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 0),
                foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 0),
                window->swapchain.surfaceFormat().format, window->swapchain.extent(),
                VK_IMAGE_LAYOUT_UNDEFINED, true, {}, &renderTargetColourImageResource);
            if (errC) {
                ERRC_END_PROGRAM
            }

            errC = foeGfxVkImportImageRenderJob(
                renderGraph, "importRenderTargetDepthImage", VK_NULL_HANDLE,
                "renderTargetDepthImage",
                foeGfxVkGetRenderTargetImage(window->gfxOffscreenRenderTarget, 1),
                foeGfxVkGetRenderTargetImageView(window->gfxOffscreenRenderTarget, 1), depthFormat,
                window->swapchain.extent(), VK_IMAGE_LAYOUT_UNDEFINED, true, {},
                &renderTargetDepthImageResource);
            if (errC) {
                ERRC_END_PROGRAM
            }

            auto pCameraPool = getComponentPool<foeCameraPool>(
                pSimulationSet->componentPools.data(), pSimulationSet->componentPools.size());
            auto *pCamera = (pCameraPool->begin<1>() + pCameraPool->find(cameraID));

            RenderSceneOutputResources output;
            errC =
                renderSceneJob(renderGraph, "render3dScene", VK_NULL_HANDLE,
                               renderTargetColourImageResource, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderTargetDepthImageResource,
                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               globalMSAA, pSimulationSet, (*pCamera)->descriptor, output);
            if (errC) {
                ERRC_END_PROGRAM
            }

            renderTargetColourImageResource = output.colourRenderTarget;
            renderTargetDepthImageResource = output.depthRenderTarget;

            foeGfxVkRenderGraphResource presentImageResource;

            errC = foeGfxVkImportSwapchainImageRenderJob(
                renderGraph, "importPresentationImage", VK_NULL_HANDLE, "presentImage",
                window->swapchain, window->swapchain.acquiredIndex(),
                window->swapchain.image(window->swapchain.acquiredIndex()),
                window->swapchain.imageView(window->swapchain.acquiredIndex()),
                window->swapchain.surfaceFormat().format, window->swapchain.extent(),
                window->swapchain.imageReadySemaphore(), &presentImageResource);
            if (errC) {
                ERRC_END_PROGRAM
            }

            if (foeGfxVkGetRenderTargetSamples(window->gfxOffscreenRenderTarget) !=
                VK_SAMPLE_COUNT_1_BIT) {
                // Resolve
                ResolveJobUsedResources resources;
                errC = foeGfxVkResolveImageRenderJob(
                    renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                    renderTargetColourImageResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, presentImageResource,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resources);
                if (errC) {
                    ERRC_END_PROGRAM
                }

                renderTargetColourImageResource = resources.srcImage;
                presentImageResource = resources.dstImage;
            } else {
                // Copy
                BlitJobUsedResources resources;
                errC = foeGfxVkBlitImageRenderJob(
                    renderGraph, "resolveRenderedImageToBackbuffer", VK_NULL_HANDLE,
                    renderTargetColourImageResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, presentImageResource,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &resources);
                if (errC) {
                    ERRC_END_PROGRAM
                }

                renderTargetColourImageResource = resources.srcImage;
                presentImageResource = resources.dstImage;
            }

#ifdef EDITOR_MODE
            errC = foeImGuiVkRenderUiJob(renderGraph, "RenderImGuiPass", VK_NULL_HANDLE,
                                         presentImageResource, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &imguiRenderer,
                                         &imguiState, frameIndex, &presentImageResource);
            if (errC) {
                ERRC_END_PROGRAM
            }
#endif
            // This is called so that the swapchain advances it's internal acquired index, as if it
            // was presented
            VkSwapchainKHR swapchain2;
            uint32_t index;
            window->swapchain.presentData(&swapchain2, &index);

            errC = foeGfxVkPresentSwapchainImageRenderJob(renderGraph, "presentFinalImage",
                                                          frameData[frameIndex].frameComplete,
                                                          presentImageResource);
            if (errC) {
                ERRC_END_PROGRAM
            }

            errC = foeGfxVkExecuteRenderGraph(renderGraph, gfxSession, gfxDelayedDestructor);
            if (errC) {
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