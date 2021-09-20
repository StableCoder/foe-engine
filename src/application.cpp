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
#include "export_yaml.hpp"
#include "graphics.hpp"
#include "log.hpp"
#include "logging.hpp"
#include "position_descriptor_pool.hpp"
#include "register_basic_functionality.hpp"
#include "render_state_pool.hpp"
#include "vk_animation.hpp"

#ifdef FOE_XR_SUPPORT
#include <foe/xr/core.hpp>
#include <foe/xr/error_code.hpp>
#include <foe/xr/openxr/runtime.hpp>

#include "xr.hpp"
#endif

#ifdef EDITOR_MODE
#include "imgui/register.hpp"
#endif

#ifdef WSI_LOADER
#include <foe/wsi/loader.h>
#endif

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

#define XR_END_PROGRAM_TUPLE                                                                       \
    {                                                                                              \
        std::error_code errC = xrRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with OpenXR error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        return std::make_tuple(false, errC.value());                                               \
    }

#define XR_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = xrRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with OpenXR error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        return errC.value();                                                                       \
    }

#define VK_END_PROGRAM_TUPLE                                                                       \
    {                                                                                              \
        std::error_code errC = vkRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with Vulkan error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        return std::make_tuple(false, errC.value());                                               \
    }

#define VK_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = vkRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with Vulkan error {}", __FILE__, __LINE__,  \
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

    foeSimulationState *pNewSimulationSet{nullptr};
    errC = importState("persistent", &searchPaths, asyncTaskFunc, &pNewSimulationSet);
    if (errC) {
        FOE_LOG(General, Fatal, "Error importing '{}' state with error: {} - {}", "persistent",
                errC.value(), errC.message())
        return std::make_tuple(false, errC.value());
    }

    std::unique_ptr<foeSimulationState, std::function<void(foeSimulationState *)>> tempSimState{
        pNewSimulationSet, [](foeSimulationState *ptr) { foeDestroySimulation(ptr); }};
    pSimulationSet = std::move(tempSimState);

    // Special Entities
    cameraID = pSimulationSet->pEntityNameMap->find("camera");

#ifdef EDITOR_MODE
    errC = registerImGui(&imguiRegistrar);
    if (errC)
        ERRC_END_PROGRAM_TUPLE

    foeLogger::instance()->registerSink(&devConsole);

    imguiState.addUI(&devConsole);
    imguiState.addUI(&fileTermination);
    imguiState.addUI(&viewFrameTimeInfo);
    imguiState.addUI(&windowInfo);

    // Per SimState UI
    pSimGroupDataUI.reset(new foeSimulationImGuiGroupData{pSimulationSet.get()});
    imguiState.addUI(pSimGroupDataUI.get());

    pEntityListUI.reset(new foeImGuiEntityList{pSimulationSet.get(), &imguiRegistrar});
    imguiState.addUI(pEntityListUI.get());

    pResourceListUI.reset(new foeImGuiResourceList{pSimulationSet.get(), &imguiRegistrar});
    imguiState.addUI(pResourceListUI.get());
#endif

#ifdef WSI_LOADER
    std::string wsiImplementation = DEFAULT_WSI_IMPLEMENTATION;
    if (!settings.window.implementation.empty())
        wsiImplementation = settings.window.implementation;

    if (!foeWsiLoadImplementation(wsiImplementation.data()))
        END_PROGRAM_TUPLE
#endif

    VkResult vkRes{VK_SUCCESS};
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
        vkRes = it.create(foeGfxVkGetDevice(gfxSession));
        if (vkRes != VK_SUCCESS) {
            VK_END_PROGRAM_TUPLE
        }
    }

    { // Initialize simulation
        foeSimulationInitInfo simInitInfo{
            .gfxSession = gfxSession,
            .externalFileSearchFn = std::bind(&foeGroupData::findExternalFile,
                                              &pSimulationSet->groupData, std::placeholders::_1),
            .asyncJobFn = asyncTaskFunc,
        };
        foeInitializeSimulation(pSimulationSet.get(), &simInitInfo);
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
    // Initialize XR if an HMD is available
    if (xrRuntime != FOE_NULL_HANDLE) {
        XrResult xrRes{XR_SUCCESS};

        errC = createXrSession(xrRuntime, gfxSession, &xrSession);
        if (errC) {
            ERRC_END_PROGRAM_TUPLE
        }

        // Session Views
        uint32_t viewConfigViewCount;
        xrRes =
            xrEnumerateViewConfigurationViews(foeXrOpenGetInstance(xrRuntime), xrSession.systemId,
                                              xrSession.type, 0, &viewConfigViewCount, nullptr);
        if (xrRes != XR_SUCCESS) {
            XR_END_PROGRAM_TUPLE
        }

        std::vector<XrViewConfigurationView> viewConfigs;
        viewConfigs.resize(viewConfigViewCount);

        xrRes = xrEnumerateViewConfigurationViews(
            foeXrOpenGetInstance(xrRuntime), xrSession.systemId, xrSession.type, viewConfigs.size(),
            &viewConfigViewCount, viewConfigs.data());
        if (xrRes != XR_SUCCESS) {
            XR_END_PROGRAM_TUPLE
        }
        xrViews.clear();
        for (auto const &it : viewConfigs) {
            xrViews.emplace_back(foeXrVkSessionView{.viewConfig = it});
        }

        // OpenXR Swapchains
        std::vector<int64_t> swapchainFormats;
        xrRes = foeXrEnumerateSwapchainFormats(xrSession.session, swapchainFormats);
        if (xrRes != XR_SUCCESS) {
            XR_END_PROGRAM_TUPLE
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
                ERRC_END_PROGRAM_TUPLE
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

            xrRes = xrCreateSwapchain(xrSession.session, &swapchainCI, &view.swapchain);
            if (xrRes != XR_SUCCESS) {
                XR_END_PROGRAM_TUPLE
            }

            // Images
            xrRes = foeXrEnumerateSwapchainVkImages(view.swapchain, view.images);
            if (xrRes != XR_SUCCESS) {
                XR_END_PROGRAM_TUPLE
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
                VkResult vkRes =
                    vkCreateImageView(foeGfxVkGetDevice(gfxSession), &viewCI, nullptr, &vkView);
                if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM_TUPLE
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
                VkResult vkRes = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI,
                                                     nullptr, &newFramebuffer);
                if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM_TUPLE
                }

                view.framebuffers.emplace_back(newFramebuffer);
            }
        }

        // OpenXR Session Begin

        { // Wait for the session state to be ready
            XrEventDataBuffer event;
            errC = foeXrOpenPollEvent(xrRuntime, event);
            if (errC == XR_EVENT_UNAVAILABLE) {
                // No events currently
            } else if (errC) {
                // Error
                ERRC_END_PROGRAM_TUPLE
            } else {
                // Got an event, process it
                switch (event.type) {
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                    XrEventDataSessionStateChanged const *stateChanged =
                        reinterpret_cast<XrEventDataSessionStateChanged *>(&event);
                    if (stateChanged->state == XR_SESSION_STATE_READY &&
                        stateChanged->session == xrSession.session) {
                        goto SESSION_READY;
                    }
                } break;

                default:
                    break;
                }
            }
        }

        vkRes = xrVkCameraSystem.initialize(gfxSession);
        if (vkRes) {
            VK_END_PROGRAM_TUPLE
        }

    SESSION_READY:
        errC = xrSession.beginSession();
        if (errC) {
            ERRC_END_PROGRAM_TUPLE
        }
    }

    // If the user specified to force an XR session and couldn't find/create the session, fail out
    if (settings.xr.forceXr && xrSession.session == XR_NULL_HANDLE) {
        FOE_LOG(General, Fatal, "XR support enabled but no HMD session was detected/started.")
        return std::make_tuple(false, 1);
    }
#endif

    return std::make_tuple(true, 0);
}

void Application::deinitialize() {
    std::error_code errC;

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxWaitIdle(gfxSession);

    exportYaml(pSimulationSet.get());

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
        foeDeinitializeSimulation(pSimulationSet.get());

#ifdef FOE_XR_SUPPORT
    // XR Cleanup
    if (xrSession.session != XR_NULL_HANDLE) {
        xrSession.requestExitSession();
        while (true) {
            XrEventDataBuffer event;
            auto errC = foeXrOpenPollEvent(xrRuntime, event);
            if (errC == XR_EVENT_UNAVAILABLE) {
                // No events currently
            } else if (errC) {
                // Error
            } else {
                // Got an event, process it
                switch (event.type) {
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                    XrEventDataSessionStateChanged const *stateChanged =
                        reinterpret_cast<XrEventDataSessionStateChanged *>(&event);
                    if (stateChanged->state == XR_SESSION_STATE_STOPPING &&
                        stateChanged->session == xrSession.session) {
                        goto SESSION_END;
                    }
                } break;

                default:
                    break;
                }
            }
        }
    SESSION_END:
        xrSession.endSession();

        while (true) {
            XrEventDataBuffer event;
            auto errC = foeXrOpenPollEvent(xrRuntime, event);
            if (errC == XR_EVENT_UNAVAILABLE) {
                // No events currently
            } else if (errC) {
                // Error
            } else {
                // Got an event, process it
                switch (event.type) {
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                    XrEventDataSessionStateChanged const *stateChanged =
                        reinterpret_cast<XrEventDataSessionStateChanged *>(&event);
                    if (stateChanged->state == XR_SESSION_STATE_IDLE &&
                        stateChanged->session == xrSession.session) {
                        goto SESSION_IDLE;
                    }
                } break;

                default:
                    break;
                }
            }
        }
    SESSION_IDLE:

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

        while (true) {
            XrEventDataBuffer event;
            auto errC = foeXrOpenPollEvent(xrRuntime, event);
            if (errC == XR_EVENT_UNAVAILABLE) {
                // No events currently
            } else if (errC) {
                // Error
            } else {
                // Got an event, process it
                switch (event.type) {
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                    XrEventDataSessionStateChanged const *stateChanged =
                        reinterpret_cast<XrEventDataSessionStateChanged *>(&event);
                    if (stateChanged->state == XR_SESSION_STATE_EXITING &&
                        stateChanged->session == xrSession.session) {
                        goto SESSION_EXITING;
                    }
                } break;

                default:
                    break;
                }
            }
        }
    SESSION_EXITING:

        xrSession.destroySession();
    }

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

    // Deregister functionality
    deregisterBasicFunctionality();

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

int Application::mainloop() {
    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock{std::chrono::nanoseconds{0}};

    VkResult vkRes{VK_SUCCESS};
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
            if (xrSession.session != XR_NULL_HANDLE) {
                XrResult xrRes{XR_SUCCESS};

                XrFrameWaitInfo frameWaitInfo{.type = XR_TYPE_FRAME_WAIT_INFO};
                xrFrameState = XrFrameState{.type = XR_TYPE_FRAME_STATE};
                xrRes = xrWaitFrame(xrSession.session, &frameWaitInfo, &xrFrameState);
                if (xrRes != XR_SUCCESS) {
                    XR_END_PROGRAM
                }
            }
#endif

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
                vkRes = it.swapchain.acquireNextImage(foeGfxVkGetDevice(gfxSession));
                if (vkRes == VK_TIMEOUT || vkRes == VK_NOT_READY) {
                    // Waiting for an image to become ready
                } else if (vkRes == VK_ERROR_OUT_OF_DATE_KHR) {
                    // Surface changed, need to rebuild swapchains
                    it.swapchain.needRebuild();
                } else if (vkRes == VK_SUBOPTIMAL_KHR) {
                    // Surface is still usable, but should rebuild next time
                    it.swapchain.needRebuild();
                    windowRenderList.emplace_back(&it);
                } else if (vkRes != VK_SUCCESS) {
                    // Catastrophic error
                    VK_END_PROGRAM
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
            if (xrSession.session != XR_NULL_HANDLE) {
                XrResult xrRes{XR_SUCCESS};

                XrFrameBeginInfo frameBeginInfo{.type = XR_TYPE_FRAME_BEGIN_INFO};
                xrRes = xrBeginFrame(xrSession.session, &frameBeginInfo);
                if (xrRes != XR_SUCCESS) {
                    XR_END_PROGRAM
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
                    xrRes = xrLocateViews(xrSession.session, &viewLocateInfo, &viewState,
                                          views.size(), &viewCountOutput, views.data());
                    if (xrRes != XR_SUCCESS) {
                        XR_END_PROGRAM
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
                        xrRes = xrAcquireSwapchainImage(it.swapchain, &acquireInfo, &newIndex);
                        if (xrRes != XR_SUCCESS) {
                            XR_END_PROGRAM
                        }

                        XrSwapchainImageWaitInfo waitInfo{
                            .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
                            .timeout = 1,
                        };
                        xrRes = xrWaitSwapchainImage(it.swapchain, &waitInfo);
                        if (xrRes != XR_SUCCESS) {
                            XR_END_PROGRAM
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

                        // Vulkan Rendering BEGIN
                        {
                            VkCommandBuffer &commandBuffer =
                                frameData[frameIndex].xrCommandBuffers[i];

                            VkCommandBufferBeginInfo commandBufferBI{
                                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                            };

                            vkRes = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                            if (vkRes != VK_SUCCESS)
                                VK_END_PROGRAM

                            VkExtent2D swapchainExtent{
                                .width = it.viewConfig.recommendedImageRectWidth,
                                .height = it.viewConfig.recommendedImageRectHeight,
                            };
                            { // Setup common render viewport data
                                VkViewport viewport{
                                    .width =
                                        static_cast<float>(it.viewConfig.recommendedImageRectWidth),
                                    .height = static_cast<float>(
                                        it.viewConfig.recommendedImageRectHeight),
                                    .minDepth = 0.f,
                                    .maxDepth = 1.f,
                                };
                                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                                VkRect2D scissor{
                                    .offset = VkOffset2D{},
                                    .extent = swapchainExtent,
                                };
                                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                                // vkDepthBias ??
                            }

                            { // Render Pass Setup
                                std::array<VkClearValue, 2> clearValues{
                                    VkClearValue{.color = {1.f, 0.5f, 1.f, 0.f}},
                                    VkClearValue{.depthStencil = {
                                                     .depth = 1.f,
                                                     .stencil = 0,
                                                 }}};

                                VkRenderPassBeginInfo renderPassBI{
                                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                    .renderPass = xrOffscreenRenderPass,
                                    .framebuffer = foeGfxVkGetRenderTargetFramebuffer(renderTarget),
                                    .renderArea =
                                        {
                                            .offset = {0, 0},
                                            .extent = swapchainExtent,
                                        },
                                    .clearValueCount = clearValues.size(),
                                    .pClearValues = clearValues.data(),
                                };

                                vkCmdBeginRenderPass(commandBuffer, &renderPassBI,
                                                     VK_SUBPASS_CONTENTS_INLINE);

                                auto *pRenderStatePool = getComponentPool<foeRenderStatePool>(
                                    pSimulationSet->componentPools.data(),
                                    pSimulationSet->componentPools.size());

                                auto idIt = pRenderStatePool->cbegin();
                                auto const endIdIt = pRenderStatePool->cend();
                                auto dataIt = pRenderStatePool->cbegin<1>();
                                for (; idIt != endIdIt; ++idIt, ++dataIt) {
                                    renderCall(*idIt, dataIt, gfxSession, pSimulationSet.get(),
                                               commandBuffer,
                                               foeGfxVkGetRenderTargetSamples(renderTarget),
                                               xrOffscreenRenderPass, it.camera.descriptor);
                                }

                                vkCmdEndRenderPass(commandBuffer);
                            }

                            { // Blit or resolve to swapchain
                                VkImageSubresourceRange subresourceRange{
                                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .layerCount = 1,
                                };

                                VkImageMemoryBarrier imgMemBarrier{
                                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                    .srcAccessMask = 0,
                                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .image = it.images[newIndex].image,
                                    .subresourceRange = subresourceRange,
                                };

                                vkCmdPipelineBarrier(commandBuffer,
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                                                     nullptr, 0, nullptr, 1, &imgMemBarrier);

                                if (foeGfxVkGetRenderTargetSamples(renderTarget) !=
                                    VK_SAMPLE_COUNT_1_BIT) {
                                    VkImageResolve resolveRegion{
                                        .srcSubresource =
                                            VkImageSubresourceLayers{
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            },
                                        .srcOffset = {},
                                        .dstSubresource =
                                            VkImageSubresourceLayers{
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            },
                                        .dstOffset = {},
                                        .extent =
                                            {
                                                .width = it.viewConfig.recommendedImageRectWidth,
                                                .height = it.viewConfig.recommendedImageRectHeight,
                                                .depth = 1,
                                            },
                                    };

                                    vkCmdResolveImage(commandBuffer,
                                                      foeGfxVkGetRenderTargetImage(renderTarget, 0),
                                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                      it.images[newIndex].image,
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                      &resolveRegion);
                                } else {
                                    VkImageCopy imageCopy{
                                        .srcSubresource =
                                            VkImageSubresourceLayers{
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            },
                                        .srcOffset = {},
                                        .dstSubresource =
                                            VkImageSubresourceLayers{
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            },
                                        .dstOffset = {},
                                        .extent =
                                            {
                                                .width = it.viewConfig.recommendedImageRectWidth,
                                                .height = it.viewConfig.recommendedImageRectHeight,
                                                .depth = 1,
                                            },
                                    };

                                    vkCmdCopyImage(commandBuffer,
                                                   foeGfxVkGetRenderTargetImage(renderTarget, 0),
                                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                   it.images[newIndex].image,
                                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                   &imageCopy);
                                }

                                imgMemBarrier = VkImageMemoryBarrier{
                                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .image = it.images[newIndex].image,
                                    .subresourceRange = subresourceRange,
                                };

                                vkCmdPipelineBarrier(commandBuffer,
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                                                     nullptr, 0, nullptr, 1, &imgMemBarrier);
                            }

                            vkRes = vkEndCommandBuffer(commandBuffer);
                            if (vkRes != VK_SUCCESS) {
                                FOE_LOG(General, Fatal, "Error with drawing: {}",
                                        std::error_code{vkRes}.message());
                            }

                            // Submission
                            VkSubmitInfo submitInfo{
                                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .commandBufferCount = 1,
                                .pCommandBuffers = &commandBuffer,
                            };

                            auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
                            vkRes = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
                            foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
                            if (vkRes != VK_SUCCESS)
                                VK_END_PROGRAM
                        }

                        // Vulkan Rendering END
                        XrSwapchainImageReleaseInfo releaseInfo{
                            .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
                        xrReleaseSwapchainImage(it.swapchain, &releaseInfo);
                        if (xrRes != XR_SUCCESS) {
                            XR_END_PROGRAM
                        }
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
                xrRes = xrEndFrame(xrSession.session, &endFrameInfo);
                if (xrRes != XR_SUCCESS) {
                    XR_END_PROGRAM
                }
            }
#endif

            // Rendering to windows
            VkCommandBuffer &commandBuffer = frameData[frameIndex].commandBuffer;

            VkCommandBufferBeginInfo commandBufferBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };

            vkRes = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
            if (vkRes != VK_SUCCESS) {
                VK_END_PROGRAM
            }

            for (auto &it : windowRenderList) {
                errC = foeGfxAcquireNextRenderTarget(it->gfxOffscreenRenderTarget,
                                                     FOE_GRAPHICS_MAX_BUFFERED_FRAMES);
                if (errC) {
                    ERRC_END_PROGRAM
                }

                { // Render offscreen
                    VkExtent2D swapchainExtent = it->swapchain.extent();

                    { // Setup common render viewport data
                        VkViewport viewport{
                            .width = static_cast<float>(swapchainExtent.width),
                            .height = static_cast<float>(swapchainExtent.height),
                            .minDepth = 0.f,
                            .maxDepth = 1.f,
                        };
                        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                        VkRect2D scissor{
                            .offset = VkOffset2D{},
                            .extent = swapchainExtent,
                        };
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                        // vkDepthBias ??
                    }

                    { // Render to Image
                        VkRenderPass renderPass =
                            foeGfxVkGetRenderPassPool(gfxSession)
                                ->renderPass(
                                    {VkAttachmentDescription{
                                         .format = it->swapchain.surfaceFormat().format,
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

                        std::array<VkClearValue, 2> clearValues{
                            VkClearValue{.color = {1.f, 0.5f, 1.f, 0.f}},
                            VkClearValue{.depthStencil =
                                             {
                                                 .depth = 1.f,
                                                 .stencil = 0,
                                             }},
                        };

                        VkRenderPassBeginInfo renderPassBI{
                            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                            .renderPass = renderPass,
                            .framebuffer =
                                foeGfxVkGetRenderTargetFramebuffer(it->gfxOffscreenRenderTarget),
                            .renderArea =
                                {
                                    .offset = {0, 0},
                                    .extent = it->swapchain.extent(),
                                },
                            .clearValueCount = clearValues.size(),
                            .pClearValues = clearValues.data(),
                        };

                        vkCmdBeginRenderPass(commandBuffer, &renderPassBI,
                                             VK_SUBPASS_CONTENTS_INLINE);

                        auto pCameraPool =
                            getComponentPool<foeCameraPool>(pSimulationSet->componentPools.data(),
                                                            pSimulationSet->componentPools.size());

                        auto *pCamera = (pCameraPool->begin<1>() + pCameraPool->find(cameraID));

                        auto *pRenderStatePool = getComponentPool<foeRenderStatePool>(
                            pSimulationSet->componentPools.data(),
                            pSimulationSet->componentPools.size());

                        auto idIt = pRenderStatePool->cbegin();
                        auto const endIdIt = pRenderStatePool->cend();
                        auto dataIt = pRenderStatePool->cbegin<1>();
                        for (; idIt != endIdIt; ++idIt, ++dataIt) {
                            renderCall(*idIt, dataIt, gfxSession, pSimulationSet.get(),
                                       commandBuffer, globalMSAA, renderPass,
                                       (*pCamera)->descriptor);
                        }

                        vkCmdEndRenderPass(commandBuffer);
                    }

                    { // Resolve to Swap Image
                        VkImageSubresourceRange subresourceRange{
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .layerCount = 1,
                        };

                        VkImageMemoryBarrier imgMemBarrier{
                            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                            .srcAccessMask = 0,
                            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .image = it->swapchain.image(it->swapchain.acquiredIndex()),
                            .subresourceRange = subresourceRange,
                        };

                        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                                             nullptr, 1, &imgMemBarrier);

                        if (foeGfxVkGetRenderTargetSamples(it->gfxOffscreenRenderTarget) !=
                            VK_SAMPLE_COUNT_1_BIT) {
                            VkImageResolve resolveRegion{
                                .srcSubresource =
                                    VkImageSubresourceLayers{
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1,
                                    },
                                .srcOffset = {},
                                .dstSubresource =
                                    VkImageSubresourceLayers{
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1,
                                    },
                                .dstOffset = {},
                                .extent =
                                    {
                                        .width = it->swapchain.extent().width,
                                        .height = it->swapchain.extent().height,
                                        .depth = 1,
                                    },
                            };

                            vkCmdResolveImage(
                                commandBuffer,
                                foeGfxVkGetRenderTargetImage(it->gfxOffscreenRenderTarget, 0),
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                it->swapchain.image(it->swapchain.acquiredIndex()),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
                        } else {
                            VkImageCopy imageCopy{
                                .srcSubresource =
                                    VkImageSubresourceLayers{
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1,
                                    },
                                .srcOffset = {},
                                .dstSubresource =
                                    VkImageSubresourceLayers{
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1,
                                    },
                                .dstOffset = {},
                                .extent =
                                    {
                                        .width = it->swapchain.extent().width,
                                        .height = it->swapchain.extent().height,
                                        .depth = 1,
                                    },
                            };

                            vkCmdCopyImage(
                                commandBuffer,
                                foeGfxVkGetRenderTargetImage(it->gfxOffscreenRenderTarget, 0),
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                it->swapchain.image(it->swapchain.acquiredIndex()),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
                        }

                        imgMemBarrier = VkImageMemoryBarrier{
                            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .image = it->swapchain.image(it->swapchain.acquiredIndex()),
                            .subresourceRange = subresourceRange,
                        };

                        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                                             nullptr, 1, &imgMemBarrier);

                        // Render the UI to the resolved swap image
                        // Render passes
                        VkRenderPass swapImageRenderPass =
                            foeGfxVkGetRenderPassPool(gfxSession)
                                ->renderPass({VkAttachmentDescription{
                                    .format = it->swapchain.surfaceFormat().format,
                                    .samples = VK_SAMPLE_COUNT_1_BIT,
                                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                    .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                }});

                        if (it->swapchain != framebufferSwapchain) {
                            {
                                auto framebuffersCopy = swapImageFramebuffers;
                                foeGfxAddDelayedDestructionCall(
                                    gfxDelayedDestructor, [=](foeGfxSession session) {
                                        for (auto &it : framebuffersCopy)
                                            vkDestroyFramebuffer(foeGfxVkGetDevice(session), it,
                                                                 nullptr);
                                    });
                            }
                            swapImageFramebuffers.clear();

                            int width, height;
                            foeWsiWindowGetSize(it->window, &width, &height);
                            VkImageView view;
                            VkExtent2D swapchainExtent = it->swapchain.extent();
                            VkFramebufferCreateInfo framebufferCI{
                                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                .renderPass = swapImageRenderPass,
                                .attachmentCount = 1,
                                .pAttachments = &view,
                                .width = (uint32_t)swapchainExtent.width,
                                .height = (uint32_t)swapchainExtent.height,
                                .layers = 1,
                            };

                            for (uint32_t i = 0; i < it->swapchain.chainSize(); ++i) {
                                view = it->swapchain.imageView(i);

                                VkFramebuffer framebuffer;
                                vkRes = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession),
                                                            &framebufferCI, nullptr, &framebuffer);
                                if (vkRes != VK_SUCCESS)
                                    VK_END_PROGRAM
                                swapImageFramebuffers.emplace_back(framebuffer);
                            }

                            framebufferSwapchain = it->swapchain;
                        }

                        VkExtent2D swapchainExtent = it->swapchain.extent();
                        VkClearValue clear{
                            .color = {0.f, 0.5f, 1.f, 0.f},
                        };
                        VkRenderPass renderPass = swapImageRenderPass;

                        VkRenderPassBeginInfo renderPassBI{
                            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                            .renderPass = renderPass,
                            .framebuffer = swapImageFramebuffers[it->swapchain.acquiredIndex()],
                            .renderArea =
                                {
                                    .offset = {0, 0},
                                    .extent = swapchainExtent,
                                },
                            .clearValueCount = 1,
                            .pClearValues = &clear,
                        };

                        vkCmdBeginRenderPass(commandBuffer, &renderPassBI,
                                             VK_SUBPASS_CONTENTS_INLINE);

#ifdef EDITOR_MODE
                        if (it == windowRenderList[0]) { // ImGui
                            if (!imguiRenderer.initialized()) {
                                errC = imguiRenderer.initialize(gfxSession, VK_SAMPLE_COUNT_1_BIT,
                                                                renderPass, 0);
                                if (errC) {
                                    ERRC_END_PROGRAM
                                }
                            }

                            imguiRenderer.newFrame();
                            imguiState.runUI();
                            imguiRenderer.endFrame();

                            errC =
                                imguiRenderer.update(foeGfxVkGetAllocator(gfxSession), frameIndex);
                            if (errC) {
                                ERRC_END_PROGRAM
                            }

                            imguiRenderer.draw(commandBuffer, frameIndex);
                        }
#endif

                        vkCmdEndRenderPass(commandBuffer);
                    }
                }
            }

            // Submit render buffer
            {
                vkRes = vkEndCommandBuffer(commandBuffer);
                if (vkRes != VK_SUCCESS) {
                    FOE_LOG(General, Fatal, "Error with drawing: {}",
                            std::error_code{vkRes}.message());
                }

                // Submission
                std::vector<VkPipelineStageFlags> waitMasks;
                std::vector<VkSemaphore> imageReadSemaphores;

                for (auto &it : windowRenderList) {
                    waitMasks.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    imageReadSemaphores.emplace_back(it->swapchain.imageReadySemaphore());
                }

                VkSubmitInfo submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = static_cast<uint32_t>(imageReadSemaphores.size()),
                    .pWaitSemaphores = imageReadSemaphores.data(),
                    .pWaitDstStageMask = waitMasks.data(),
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &frameData[frameIndex].renderComplete,
                };

                auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
                vkRes = vkQueueSubmit(queue, 1, &submitInfo, frameData[frameIndex].frameComplete);
                foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
                if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }

            // Presentation
            {
                std::vector<VkSwapchainKHR> swapchains;
                std::vector<uint32_t> swapchainIndices;
                std::vector<VkResult> swapchainResults;

                for (auto &it : windowRenderList) {
                    VkSwapchainKHR swapchain2;
                    uint32_t index;
                    it->swapchain.presentData(&swapchain2, &index);

                    swapchains.emplace_back(swapchain2);
                    swapchainIndices.emplace_back(index);
                    swapchainResults.emplace_back(VK_SUCCESS);
                }

                VkPresentInfoKHR presentInfo{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &frameData[frameIndex].renderComplete,
                    .swapchainCount = static_cast<uint32_t>(swapchains.size()),
                    .pSwapchains = swapchains.data(),
                    .pImageIndices = swapchainIndices.data(),
                    .pResults = swapchainResults.data(),
                };

                auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
                vkRes = vkQueuePresentKHR(queue, &presentInfo);
                foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
                if (vkRes == VK_ERROR_OUT_OF_DATE_KHR || vkRes == VK_SUBOPTIMAL_KHR) {
                    // The associated window has been resized, will be fixed for the next frame
                    vkRes = VK_SUCCESS;
                } else if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
                for (auto &it : swapchainResults) {
                    if (it == VK_ERROR_OUT_OF_DATE_KHR || it == VK_SUBOPTIMAL_KHR) {
                        // The associated window has been resized, will be fixed for the next frame
                        it = VK_SUCCESS;
                    } else if (it != VK_SUCCESS) {
                        VK_END_PROGRAM
                    }
                }
            }

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