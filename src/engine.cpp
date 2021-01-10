/*
    Copyright (C) 2020 George Cave.

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

#include <GLFW/glfw3.h>
#include <foe/chrono/dilated_long_clock.hpp>
#include <foe/chrono/program_clock.hpp>
#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/descriptor_set_layout_pool.hpp>
#include <foe/graphics/fragment_descriptor.hpp>
#include <foe/graphics/fragment_descriptor_pool.hpp>
#include <foe/graphics/render_pass_pool.hpp>
#include <foe/graphics/resource_uploader.hpp>
#include <foe/graphics/shader_pool.hpp>
#include <foe/graphics/swapchain.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/vertex_descriptor.hpp>
#include <foe/graphics/vk/pipeline_pool.hpp>
#include <foe/graphics/vk/runtime.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/log.hpp>
#include <foe/quaternion_math.hpp>
#include <foe/wsi.hpp>
#include <foe/wsi_vulkan.hpp>
#include <foe/xr/core.hpp>
#include <foe/xr/debug_utils.hpp>
#include <foe/xr/error_code.hpp>
#include <foe/xr/runtime.hpp>
#include <foe/xr/session.hpp>
#include <foe/xr/vulkan.hpp>
#include <vk_error_code.hpp>

#include <array>
#include <chrono>

#include "camera.hpp"
#include "camera_descriptor_pool.hpp"
#include "frame_timer.hpp"
#include "logging.hpp"
#include "settings.hpp"
#include "vulkan_setup.hpp"
#include "xr_camera.hpp"
#include "xr_setup.hpp"

#ifdef EDITOR_MODE
#include <foe/imgui/renderer.hpp>
#include <foe/imgui/state.hpp>
#include <imgui.h>

#include "imgui/frame_time_info.hpp"
#include "imgui/termination.hpp"
#endif

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,         \
                errC.message());                                                                   \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

#define XR_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = xrRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with OpenXR error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

#define VK_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = res;                                                                \
        FOE_LOG(General, Fatal, "End called from {}:{} with Vulkan error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{}", __FILE__, __LINE__);                      \
        goto SHUTDOWN_PROGRAM;                                                                     \
    }

using namespace std::chrono_literals;

struct VkWindowData {
    VkSurfaceKHR surface{VK_NULL_HANDLE};
};

void destroyVkWindowData(VkWindowData *pData, VkInstance instance, VkDevice device) {
    if (pData->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, pData->surface, nullptr);
}

struct PerFrameData {
    VkSemaphore presentImageAcquired;
    VkSemaphore renderComplete;

    VkFence frameComplete;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    std::array<VkCommandBuffer, 2> xrCommandBuffers;

    VkResult create(VkDevice device) noexcept {
        // Semaphores
        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        auto res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentImageAcquired);
        if (res != VK_SUCCESS)
            return res;

        res = vkCreateSemaphore(device, &semaphoreCI, nullptr, &renderComplete);
        if (res != VK_SUCCESS)
            return res;

        // Fences
        VkFenceCreateInfo fenceCI{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        res = vkCreateFence(device, &fenceCI, nullptr, &frameComplete);
        if (res != VK_SUCCESS)
            return res;

        // Command Pools
        VkCommandPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        };
        res = vkCreateCommandPool(device, &poolCI, nullptr, &commandPool);
        if (res != VK_SUCCESS)
            return res;

        // Command Buffers
        VkCommandBufferAllocateInfo commandBufferAI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        res = vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);
        if (res != VK_SUCCESS)
            return res;

        commandBufferAI.commandBufferCount = 2;
        res = vkAllocateCommandBuffers(device, &commandBufferAI, xrCommandBuffers.data());
        if (res != VK_SUCCESS)
            return res;

        return res;
    }

    void destroy(VkDevice device) {
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyFence(device, frameComplete, nullptr);

        vkDestroySemaphore(device, renderComplete, nullptr);
        vkDestroySemaphore(device, presentImageAcquired, nullptr);
    }
};

void processUserInput(double timeElapsedInSeconds,
                      foeKeyboard const *pKeyboard,
                      foeMouse const *pMouse,
                      Camera *pCamera) {
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

int main(int argc, char **argv) {
    initializeLogging();

    // Settings
    Settings settings;
    if (auto retVal = loadSettings(argc, argv, settings); retVal != 0) {
        return retVal;
    }

    foeEasyProgramClock programClock;
    foeDilatedLongClock simulationClock(std::chrono::nanoseconds{0});

    FrameTimer frameTime;

    foeXrRuntime xrRuntime;

    struct foeXrSessionView {
        XrViewConfigurationView viewConfig;
        XrSwapchain swapchain;
        VkFormat format;
        std::vector<XrSwapchainImageVulkanKHR> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;
        foeXrCamera camera;
    };

    foeXrSession xrSession;
    VkRenderPass xrRenderPass;
    std::vector<foeXrSessionView> xrViews;

    foeGfxRuntime gfxRuntime{FOE_NULL_HANDLE};
    foeGfxSession gfxSession{FOE_NULL_HANDLE};

    foeResourceUploader resUploader;

    VkWindowData vkWindow;
    foeSwapchain swapchain;

    uint32_t frameIndex = 0;
    std::array<PerFrameData, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> frameData;

    foeVertexDescriptor vertexDescriptor;
    foeFragmentDescriptor *fragmentDescriptor{nullptr};

    foeRenderPassPool renderPassPool;

    foeDescriptorSetLayoutPool descriptorSetLayoutPool;
    foeBuiltinDescriptorSets builtinDescriptorSets;
    foeShaderPool shaderPool;
    foeFragmentDescriptorPool fragmentDescriptorPool;
    foeGfxVkPipelinePool pipelinePool;

    Camera camera;
    CameraDescriptorPool cameraDescriptorPool;

    cameraDescriptorPool.linkCamera(&camera);

#ifdef EDITOR_MODE
    foeImGuiRenderer imguiRenderer;
    foeImGuiState imguiState;

    foeImGuiTermination fileTermination;
    foeImGuiFrameTimeInfo viewFrameTimeInfo{&frameTime};

    imguiState.addUI(&fileTermination);
    imguiState.addUI(&viewFrameTimeInfo);
#endif

    std::vector<VkFramebuffer> swapImageFramebuffers;
    bool swapchainRebuilt = false;

    XrResult xrRes{XR_SUCCESS};
    VkResult res{VK_SUCCESS};

    {
        // Create Windows
        // Determine OpenXR layers/extensions
        // OpenXR INIT (& debug callback)
        // Determine Vulkan instance layers/extensions
        // VkInstance INIT (& debug callback)
        // Get Window VkSurface's
        // Determine Vk Physical device (OpenXR requirements, VkSurface Reqs, etc)
        // Determine Vk device layers/extensions
        // VkDevice INIT
        // XrSession INIT

        if (!foeCreateWindow(settings.window.width, settings.window.height, "FoE Engine", true)) {
            END_PROGRAM
        }

        auto [layers, extensions] = determineXrInstanceEnvironment(settings.xr.debugLogging);
        auto errC =
            xrRuntime.createRuntime("FoE Engine", 0, layers, extensions, settings.xr.debugLogging);
        if (errC && settings.xr.forceXr) {
            ERRC_END_PROGRAM
        }

        auto [instanceLayers, instanceExtensions] = determineVkInstanceEnvironment(
            xrRuntime.instance, settings.window.enableWSI, settings.graphics.validation,
            settings.graphics.debugLogging);

        errC = foeGfxVkCreateRuntime("FoE Engine", 0, instanceLayers, instanceExtensions,
                                     settings.graphics.validation, settings.graphics.debugLogging,
                                     &gfxRuntime);
        if (errC) {
            ERRC_END_PROGRAM
        }

        res = foeWindowGetVkSurface(foeGfxVkGetInstance(gfxRuntime), &vkWindow.surface);
        if (res != VK_SUCCESS) {
            VK_END_PROGRAM
        }

        VkPhysicalDevice vkPhysicalDevice =
            determineVkPhysicalDevice(foeGfxVkGetInstance(gfxRuntime), xrRuntime.instance,
                                      vkWindow.surface, settings.graphics.gpu, settings.xr.forceXr);
        auto [deviceLayers, deviceExtensions] =
            determineVkDeviceEnvironment(xrRuntime.instance, vkWindow.surface != VK_NULL_HANDLE);

        errC = foeGfxVkCreateSession(gfxRuntime, vkPhysicalDevice, deviceLayers, deviceExtensions,
                                     &gfxSession);
        if (errC) {
            ERRC_END_PROGRAM
        }
    }

    res = foeGfxCreateResourceUploader(gfxSession, &resUploader);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

        {
            camera.viewX = settings.window.width;
            camera.viewY = settings.window.height;
            camera.fieldOfViewY = 60.f;
            camera.nearZ = 2.f;
            camera.farZ = 50.f;

            camera.position = glm::vec3(0.f, 0.f, -5.f);
            camera.orientation = glm::quat(glm::vec3(0, 0, 0));
        }

#ifdef EDITOR_MODE
    imguiRenderer.resize(settings.window.width, settings.window.height);
    float xScale, yScale;
    foeWindowGetContentScale(&xScale, &yScale);
    imguiRenderer.rescale(xScale, yScale);
#endif

    for (auto &it : frameData) {
        res = it.create(foeGfxVkGetDevice(gfxSession));
        if (res != VK_SUCCESS) {
            VK_END_PROGRAM
        }
    }

    res = renderPassPool.initialize(foeGfxVkGetDevice(gfxSession));
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = descriptorSetLayoutPool.initialize(foeGfxVkGetDevice(gfxSession));
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = builtinDescriptorSets.initialize(foeGfxVkGetDevice(gfxSession), &descriptorSetLayoutPool);
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = shaderPool.initialize(foeGfxVkGetDevice(gfxSession));
    if (res != VK_SUCCESS)
        VK_END_PROGRAM

    res = pipelinePool.initialize(gfxSession, &builtinDescriptorSets);
    if (res != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    res = cameraDescriptorPool.initialize(
        gfxSession,
        builtinDescriptorSets.getBuiltinLayout(
            foeBuiltinDescriptorSetLayoutFlagBits::
                FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX),
        builtinDescriptorSets.getBuiltinSetLayoutIndex(
            foeBuiltinDescriptorSetLayoutFlagBits::
                FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX));
    if (res != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    {
        foeShader *pShader;

        pShader = shaderPool.create("data/shaders/simple/camera_tri.vert.spv");
        pShader->incrementUseCount();
        pShader->builtinSetLayouts = foeBuiltinDescriptorSetLayoutFlagBits::
            FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX;

        // Vertex
        vertexDescriptor.mVertex = pShader;
        vertexDescriptor.mVertexInputSCI = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };
        vertexDescriptor.mInputAssemblySCI = VkPipelineInputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        };

        // Fragment
        pShader = shaderPool.create("data/shaders/simple/uv_to_colour.frag.spv");
        pShader->incrementUseCount();

        auto rasterization = VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .lineWidth = 1.0f,
        };
        std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments{
            VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            }};
        auto colourBlend = VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(colourBlendAttachments.size()),
            .pAttachments = colourBlendAttachments.data(),
        };

        fragmentDescriptor =
            fragmentDescriptorPool.get(&rasterization, nullptr, &colourBlend, pShader);
    }

    if (xrRuntime.instance != XR_NULL_HANDLE) {
        XrSystemId xrSystemId{};

        // OpenXR SystemId
        if (xrRuntime.instance != XR_NULL_HANDLE) {
            XrSystemGetInfo systemGetInfo{
                .type = XR_TYPE_SYSTEM_GET_INFO,
                .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
            };

            xrRes = xrGetSystem(xrRuntime.instance, &systemGetInfo, &xrSystemId);
            if (xrRes != XR_SUCCESS) {
                XR_END_PROGRAM
            }
        }

        // Types
        uint32_t viewConfigCount;
        xrRes = xrEnumerateViewConfigurations(xrRuntime.instance, xrSystemId, 0, &viewConfigCount,
                                              nullptr);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }

        std::vector<XrViewConfigurationType> xrViewConfigTypes;
        xrViewConfigTypes.resize(viewConfigCount);

        xrRes =
            xrEnumerateViewConfigurations(xrRuntime.instance, xrSystemId, xrViewConfigTypes.size(),
                                          &viewConfigCount, xrViewConfigTypes.data());
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }

        // Is View Mutable??
        XrViewConfigurationProperties xrViewConfigProps{.type =
                                                            XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
        xrRes = xrGetViewConfigurationProperties(xrRuntime.instance, xrSystemId,
                                                 xrViewConfigTypes[0], &xrViewConfigProps);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }

        // Check graphics requirements
        XrGraphicsRequirementsVulkanKHR gfxRequirements;
        xrRes =
            foeXrGetVulkanGraphicsRequirements(xrRuntime.instance, xrSystemId, &gfxRequirements);

        // XrSession
        XrGraphicsBindingVulkanKHR gfxBinding{
            .type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
            .instance = foeGfxVkGetInstance(gfxRuntime),
            .physicalDevice = foeGfxVkGetPhysicalDevice(gfxSession),
            .device = foeGfxVkGetDevice(gfxSession),
            .queueFamilyIndex = 0,
            .queueIndex = 0,
        };
        auto errC = xrSession.createSession(xrRuntime.instance, xrSystemId, xrViewConfigTypes[0],
                                            &gfxBinding);
        if (errC && settings.xr.forceXr) {
            ERRC_END_PROGRAM
        }

        // Session Views

        // Views
        uint32_t viewConfigViewCount;
        xrRes = xrEnumerateViewConfigurationViews(xrRuntime.instance, xrSession.systemId,
                                                  xrViewConfigTypes[0], 0, &viewConfigViewCount,
                                                  nullptr);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }

        std::vector<XrViewConfigurationView> viewConfigs;
        viewConfigs.resize(viewConfigViewCount);

        xrRes = xrEnumerateViewConfigurationViews(xrRuntime.instance, xrSession.systemId,
                                                  xrViewConfigTypes[0], viewConfigs.size(),
                                                  &viewConfigViewCount, viewConfigs.data());
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }
        xrViews.clear();
        for (auto const &it : viewConfigs) {
            xrViews.emplace_back(foeXrSessionView{.viewConfig = it});
        }

        // OpenXR Swapchains
        std::vector<int64_t> swapchainFormats;
        xrRes = foeXrEnumerateSwapchainFormats(xrSession.session, swapchainFormats);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }
        for (auto &it : xrViews) {
            it.format = static_cast<VkFormat>(swapchainFormats[0]);
        }

        xrRenderPass = renderPassPool.renderPass({VkAttachmentDescription{
            .format = static_cast<VkFormat>(swapchainFormats[0]),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        }});

        for (auto &view : xrViews) {
            // Swapchain
            XrSwapchainCreateInfo swapchainCI{
                .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
                .createFlags = 0,
                .usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
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
                return xrRes;
            }

            // Images
            xrRes = foeXrEnumerateSwapchainVkImages(view.swapchain, view.images);
            if (xrRes != XR_SUCCESS) {
                return xrRes;
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
                VkResult res =
                    vkCreateImageView(foeGfxVkGetDevice(gfxSession), &viewCI, nullptr, &vkView);
                if (res != VK_SUCCESS) {
                    return res;
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
                VkResult res = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI,
                                                   nullptr, &newFramebuffer);
                if (res != VK_SUCCESS) {
                    return res;
                }

                view.framebuffers.emplace_back(newFramebuffer);
            }
        }

        for (auto &it : xrViews) {
            it.camera.startPos = camera.position;
            it.camera.nearZ = camera.nearZ;
            it.camera.farZ = camera.farZ;
            cameraDescriptorPool.linkCamera(&it.camera);
        }

        // OpenXR Session Begin

        { // Wait for the session state to be ready
            XrEventDataBuffer event;
            errC = xrRuntime.pollEvent(event);
            if (errC == XR_EVENT_UNAVAILABLE) {
                // No events currently
            } else if (errC) {
                // Error
                ERRC_END_PROGRAM
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
                }
            }
        }

    SESSION_READY:
        errC = xrSession.beginSession();
        if (errC) {
            ERRC_END_PROGRAM
        }
    }

    // MAIN LOOP BEGIN
    foeWindowShow();
    programClock.update();
    simulationClock.externalTime(programClock.currentTime<std::chrono::nanoseconds>());

    FOE_LOG(General, Info, "Entering main loop")
    while (!foeWindowGetShouldClose()
#ifdef EDITOR_MODE
           && !fileTermination.terminationRequested()
#endif
    ) {
        // Timing
        programClock.update();
        simulationClock.update(programClock.currentTime<std::chrono::nanoseconds>());
        double timeElapsedInSec = simulationClock.elapsed().count() * 0.000000001f;

        swapchainRebuilt = false;
        foeWindowEventProcessing();

        auto *pMouse = foeGetMouse();
        auto *pKeyboard = foeGetKeyboard();

#ifdef EDITOR_MODE
        // User input processing
        imguiRenderer.keyboardInput(pKeyboard);
        imguiRenderer.mouseInput(pMouse);
        if (!imguiRenderer.wantCaptureKeyboard() && !imguiRenderer.wantCaptureMouse())
#endif
        {
            processUserInput(timeElapsedInSec, pKeyboard, pMouse, &camera);
        }

        if (foeWindowResized()) {
            // Swapchins will need rebuilding
            swapchain.requestRebuild();

#ifdef EDITOR_MODE
            int width, height;
            foeWindowGetSize(&width, &height);
            imguiRenderer.resize(width, height);
#endif
        }

        // Vulkan Render Section
        uint32_t nextFrameIndex = (frameIndex + 1) % frameData.size();
        if (VK_SUCCESS == vkWaitForFences(foeGfxVkGetDevice(gfxSession), 1,
                                          &frameData[nextFrameIndex].frameComplete, VK_TRUE, 0)) {
            frameTime.newFrame();

            // Rebuild swapchains
            if (!swapchain || swapchain.needRebuild()) {
                // If no swapchain, then that means we need to get the surface format and
                // presentation mode first
                if (!swapchain) {
                    { // Surface Formats
                        uint32_t formatCount;
                        res = vkGetPhysicalDeviceSurfaceFormatsKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), vkWindow.surface, &formatCount,
                            nullptr);
                        if (res != VK_SUCCESS)
                            VK_END_PROGRAM

                        std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats(
                            new VkSurfaceFormatKHR[formatCount]);

                        res = vkGetPhysicalDeviceSurfaceFormatsKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), vkWindow.surface, &formatCount,
                            surfaceFormats.get());
                        if (res != VK_SUCCESS)
                            VK_END_PROGRAM

                        swapchain.surfaceFormat(surfaceFormats.get()[0]);
                    }

                    { // Present Modes
                        uint32_t modeCount;
                        vkGetPhysicalDeviceSurfacePresentModesKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), vkWindow.surface, &modeCount,
                            nullptr);

                        std::unique_ptr<VkPresentModeKHR[]> presentModes(
                            new VkPresentModeKHR[modeCount]);

                        vkGetPhysicalDeviceSurfacePresentModesKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), vkWindow.surface, &modeCount,
                            presentModes.get());

                        swapchain.presentMode(presentModes.get()[0]);
                    }
                }

                foeSwapchain newSwapchain;

                int width, height;
                foeWindowGetSize(&width, &height);
                res = newSwapchain.create(foeGfxVkGetPhysicalDevice(gfxSession),
                                          foeGfxVkGetDevice(gfxSession), vkWindow.surface,
                                          swapchain.surfaceFormat(), swapchain.presentMode(),
                                          swapchain, 3, width, height);
                if (res != VK_SUCCESS)
                    VK_END_PROGRAM

                // If the old swapchain exists, we need to destroy it
                if (swapchain) {
                    swapchain.destroy(foeGfxVkGetDevice(gfxSession));
                }

                swapchain = newSwapchain;
                swapchainRebuilt = true;
            }

            // Acquire Target Presentation Images
            VkResult res = swapchain.acquireNextImage(
                foeGfxVkGetDevice(gfxSession), frameData[nextFrameIndex].presentImageAcquired);
            if (res == VK_TIMEOUT || res == VK_NOT_READY) {
                // Waiting for an image to become ready
                goto SKIP_FRAME_RENDER;
            } else if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
                // Surface changed, best to rebuild swapchains
                goto SKIP_FRAME_RENDER;
            } else if (res != VK_SUCCESS) {
                // Catastrophic error
                VK_END_PROGRAM
            }
            vkResetFences(foeGfxVkGetDevice(gfxSession), 1,
                          &frameData[nextFrameIndex].frameComplete);
            frameIndex = nextFrameIndex;

            // Set camera descriptors
            bool camerasRemade = false;

            // Rendering
            vkResetCommandPool(foeGfxVkGetDevice(gfxSession), frameData[nextFrameIndex].commandPool,
                               0);

            // OpenXR Render Section
            if (xrSession.session != XR_NULL_HANDLE) {
                XrFrameWaitInfo frameWaitInfo{.type = XR_TYPE_FRAME_WAIT_INFO};
                XrFrameState frameState{.type = XR_TYPE_FRAME_STATE};
                xrRes = xrWaitFrame(xrSession.session, &frameWaitInfo, &frameState);
                if (xrRes != XR_SUCCESS) {
                    XR_END_PROGRAM
                }

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

                if (frameState.shouldRender) {
                    XrViewLocateInfo viewLocateInfo{
                        .type = XR_TYPE_VIEW_LOCATE_INFO,
                        .displayTime = frameState.predictedDisplayTime,
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

                    for (int i = 0; i < views.size(); ++i) {
                        projectionViews[i].pose = views[i].pose;
                        projectionViews[i].fov = views[i].fov;

                        xrViews[i].camera.fov = views[i].fov;
                        xrViews[i].camera.pose = views[i].pose;
                    }

                    cameraDescriptorPool.generateCameraDescriptors(frameIndex);
                    camerasRemade = true;

                    // Render Code
                    std::vector<uint32_t> swapchainIndex;
                    for (int i = 0; i < xrViews.size(); ++i) {
                        auto &it = xrViews[i];

                        XrSwapchainImageAcquireInfo acquireInfo{
                            .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};

                        uint32_t newIndex;
                        xrRes = xrAcquireSwapchainImage(it.swapchain, &acquireInfo, &newIndex);
                        if (xrRes != XR_SUCCESS) {
                            XR_END_PROGRAM
                        }
                        swapchainIndex.emplace_back(newIndex);

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

                            res = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                            if (res != VK_SUCCESS)
                                goto SHUTDOWN_PROGRAM;

                            { // Render Pass Setup
                                VkExtent2D swapchainExtent{
                                    .width = it.viewConfig.recommendedImageRectWidth,
                                    .height = it.viewConfig.recommendedImageRectHeight,
                                };
                                VkClearValue clear{
                                    .color = {0.f, 0.f, 1.f, 0.f},
                                };

                                VkRenderPassBeginInfo renderPassBI{
                                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                    .renderPass = xrRenderPass,
                                    .framebuffer = it.framebuffers[newIndex],
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

                                if (true) { // Set Drawing Parameters
                                    VkViewport viewport{
                                        .width = static_cast<float>(
                                            it.viewConfig.recommendedImageRectWidth),
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

                                    // If we had depthbias enabled
                                    // vkCmdSetDepthBias

                                    // Set the Pipeline
                                    VkPipelineLayout layout;
                                    uint32_t descriptorSetLayoutCount;
                                    VkPipeline pipeline;

                                    pipelinePool.getPipeline(&vertexDescriptor, fragmentDescriptor,
                                                             xrRenderPass, 0, &layout,
                                                             &descriptorSetLayoutCount, &pipeline);

                                    vkCmdBindDescriptorSets(
                                        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                                        1, &it.camera.descriptor, 0, nullptr);

                                    vkCmdBindPipeline(commandBuffer,
                                                      VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                                    vkCmdDraw(commandBuffer, 4, 1, 0, 0);
                                }

                                vkCmdEndRenderPass(commandBuffer);
                            }

                            res = vkEndCommandBuffer(commandBuffer);
                            if (res != VK_SUCCESS) {
                                FOE_LOG(General, Fatal, "Error with drawing: {}",
                                        std::error_code{res}.message());
                            }

                            // Submission
                            VkPipelineStageFlags waitMask =
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                            VkSubmitInfo submitInfo{
                                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .commandBufferCount = 1,
                                .pCommandBuffers = &commandBuffer,
                            };

                            res = vkQueueSubmit(getFirstQueue(gfxSession)->queue[0], 1, &submitInfo,
                                                VK_NULL_HANDLE);
                            if (res != VK_SUCCESS)
                                goto SHUTDOWN_PROGRAM;
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
                    .displayTime = frameState.predictedDisplayTime,
                    .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
                    .layerCount = static_cast<uint32_t>(layers.size()),
                    .layers = layers.data(),
                };
                xrRes = xrEndFrame(xrSession.session, &endFrameInfo);
                if (xrRes != XR_SUCCESS) {
                    XR_END_PROGRAM
                }
            }

            if (!camerasRemade)
                cameraDescriptorPool.generateCameraDescriptors(frameIndex);

            // Render passes
            VkRenderPass swapImageRenderPass = renderPassPool.renderPass({VkAttachmentDescription{
                .format = swapchain.surfaceFormat().format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            }});

#ifdef EDITOR_MODE
            if (!imguiRenderer.initialized()) {
                if (imguiRenderer.initialize(gfxSession, VK_SAMPLE_COUNT_1_BIT, swapImageRenderPass,
                                             0) != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }
#endif

            if (swapchainRebuilt) {
                for (auto &it : swapImageFramebuffers)
                    vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);
                swapImageFramebuffers.clear();

                int width, height;
                foeWindowGetSize(&width, &height);
                VkImageView view;
                VkExtent2D swapchainExtent = swapchain.extent();
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = swapImageRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = &view,
                    .width = (uint32_t)swapchainExtent.width,
                    .height = (uint32_t)swapchainExtent.height,
                    .layers = 1,
                };

                for (uint32_t i = 0; i < swapchain.chainSize(); ++i) {
                    view = swapchain.imageView(i);

                    VkFramebuffer framebuffer;
                    res = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI,
                                              nullptr, &framebuffer);
                    if (res != VK_SUCCESS)
                        VK_END_PROGRAM
                    swapImageFramebuffers.emplace_back(framebuffer);
                }
            }

            {
                VkCommandBuffer &commandBuffer = frameData[frameIndex].commandBuffer;

                VkCommandBufferBeginInfo commandBufferBI{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };

                res = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                if (res != VK_SUCCESS)
                    goto SHUTDOWN_PROGRAM;

                { // Render Pass Setup
                    VkExtent2D swapchainExtent = swapchain.extent();
                    VkClearValue clear{
                        .color = {0.f, 0.f, 1.f, 0.f},
                    };

                    VkRenderPassBeginInfo renderPassBI{
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = swapImageRenderPass,
                        .framebuffer = swapImageFramebuffers[swapchain.acquiredIndex()],
                        .renderArea =
                            {
                                .offset = {0, 0},
                                .extent = swapchainExtent,
                            },
                        .clearValueCount = 1,
                        .pClearValues = &clear,
                    };

                    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

                    { // Set Drawing Parameters
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

                        // If we had depthbias enabled
                        // vkCmdSetDepthBias

                        // Set the Pipeline
                        VkPipelineLayout layout;
                        uint32_t descriptorSetLayoutCount;
                        VkPipeline pipeline;

                        pipelinePool.getPipeline(&vertexDescriptor, fragmentDescriptor,
                                                 swapImageRenderPass, 0, &layout,
                                                 &descriptorSetLayoutCount, &pipeline);

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                layout, 0, 1, &camera.descriptor, 0, nullptr);

                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                        vkCmdDraw(commandBuffer, 4, 1, 0, 0);
                    }

#ifdef EDITOR_MODE
                    { // ImGui
                        imguiRenderer.newFrame();
                        imguiState.runUI();
                        imguiRenderer.endFrame();

                        res = imguiRenderer.update(foeGfxVkGetAllocator(gfxSession), frameIndex);
                        if (res != VK_SUCCESS) {
                            VK_END_PROGRAM
                        }

                        imguiRenderer.draw(commandBuffer, frameIndex);
                    }
#endif

                    vkCmdEndRenderPass(commandBuffer);
                }

                res = vkEndCommandBuffer(commandBuffer);
                if (res != VK_SUCCESS) {
                    FOE_LOG(General, Fatal, "Error with drawing: {}",
                            std::error_code{res}.message());
                }

                // Submission
                VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                VkSubmitInfo submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &frameData[frameIndex].presentImageAcquired,
                    .pWaitDstStageMask = &waitMask,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &frameData[frameIndex].renderComplete,
                };

                res = vkQueueSubmit(getFirstQueue(gfxSession)->queue[0], 1, &submitInfo,
                                    frameData[frameIndex].frameComplete);
                if (res != VK_SUCCESS)
                    goto SHUTDOWN_PROGRAM;
            }

            // Presentation
            {
                std::vector<VkSwapchainKHR> swapchains;
                std::vector<uint32_t> swapchainIndices;
                std::vector<VkResult> swapchainResults;

                {
                    VkSwapchainKHR swapchain2;
                    uint32_t index;
                    swapchain.presentData(&swapchain2, &index);

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

                VkResult res = vkQueuePresentKHR(getFirstQueue(gfxSession)->queue[0], &presentInfo);
                if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                    // The associated window has been resized, will be fixed for the next frame
                    res = VK_SUCCESS;
                } else if (res != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }
        }
    SKIP_FRAME_RENDER:;
    }
SHUTDOWN_PROGRAM:
    FOE_LOG(General, Info, "Exiting main loop")

    if (gfxSession != FOE_NULL_HANDLE)
        vkDeviceWaitIdle(foeGfxVkGetDevice(gfxSession));

    // OpenXR Cleanup
    if (xrSession.session != XR_NULL_HANDLE) {
        xrSession.requestExitSession();
        {
            XrEventDataBuffer event;
            auto errC = xrRuntime.pollEvent(event);
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
                }
            }
        }
    SESSION_END:
        xrSession.endSession();

        for (auto &view : xrViews) {
            for (auto it : view.imageViews) {
                vkDestroyImageView(foeGfxVkGetDevice(gfxSession), it, nullptr);
            }
            if (view.swapchain != XR_NULL_HANDLE) {
                xrDestroySwapchain(view.swapchain);
            }
        }

        xrSession.destroySession();
    }

    for (auto &it : frameData) {
        it.destroy(foeGfxVkGetDevice(gfxSession));
    }

    for (auto &it : swapImageFramebuffers)
        vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);

    cameraDescriptorPool.deinitialize();

    pipelinePool.deinitialize();
    shaderPool.deinitialize();
    builtinDescriptorSets.deinitialize(foeGfxVkGetDevice(gfxSession));
    descriptorSetLayoutPool.deinitialize();

    renderPassPool.deinitialize();

    swapchain.destroy(foeGfxVkGetDevice(gfxSession));
    destroyVkWindowData(&vkWindow, foeGfxVkGetInstance(gfxRuntime), foeGfxVkGetDevice(gfxSession));

    foeDestroyWindow();

    foeGfxDestroyResourceUploader(&resUploader);

#ifdef EDITOR_MODE
    imguiRenderer.deinitialize(gfxSession);
#endif

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxDestroySession(gfxSession);
    if (gfxRuntime != FOE_NULL_HANDLE)
        foeGfxDestroyRuntime(gfxRuntime);

    xrRuntime.destroyRuntime();

    // Output configuration settings to a YAML configuration file
    saveSettings(settings);

    return 0;
}