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
#include <foe/graphics/vk/queue_family.hpp>
#include <foe/graphics/vk/runtime.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/graphics/vk/shader.hpp>
#include <foe/log.hpp>
#include <foe/quaternion_math.hpp>
#include <foe/wsi_vulkan.hpp>

#include <vk_error_code.hpp>

#include "graphics.hpp"
#include "logging.hpp"
#include "shader/external_shader.hpp"

#ifdef FOE_XR_SUPPORT
#include <foe/xr/core.hpp>
#include <foe/xr/error_code.hpp>
#include <foe/xr/openxr/runtime.hpp>

#include "xr.hpp"
#endif

#define ERRC_END_PROGRAM                                                                           \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with error {}", __FILE__, __LINE__,         \
                errC.message());                                                                   \
        return errC.value();                                                                       \
    }

#define XR_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = xrRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with OpenXR error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        return errC.value();                                                                       \
    }

#define VK_END_PROGRAM                                                                             \
    {                                                                                              \
        std::error_code errC = vkRes;                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{} with Vulkan error {}", __FILE__, __LINE__,  \
                errC.message());                                                                   \
        return errC.value();                                                                       \
    }

#define END_PROGRAM                                                                                \
    {                                                                                              \
        FOE_LOG(General, Fatal, "End called from {}:{}", __FILE__, __LINE__);                      \
        return 1;                                                                                  \
    }

int Application::initialize(int argc, char **argv) {
    initializeLogging();

    synchronousThreadPool.start(2);
    asynchronousThreadPool.start(2);

    if (auto retVal = loadSettings(argc, argv, settings); retVal != 0) {
        return retVal;
    }

    cameraDescriptorPool.linkCamera(&camera);

#ifdef EDITOR_MODE
    imguiState.addUI(&fileTermination);
    imguiState.addUI(&viewFrameTimeInfo);
#endif

    std::error_code errC{};
    VkResult vkRes{VK_SUCCESS};
    {
        if (!foeCreateWindow(settings.window.width, settings.window.height, "FoE Engine", true)) {
            END_PROGRAM
        }

#ifdef FOE_XR_SUPPORT
        if (settings.xr.enableXr || settings.xr.forceXr) {
            errC = createXrRuntime(settings.xr.debugLogging, &xrRuntime);
            if (errC && settings.xr.forceXr) {
                ERRC_END_PROGRAM
            }
        }
#endif

        errC = createGfxRuntime(xrRuntime, settings.window.enableWSI, settings.graphics.validation,
                                settings.graphics.debugLogging, &gfxRuntime);
        if (errC) {
            ERRC_END_PROGRAM
        }

        vkRes = foeWindowGetVkSurface(foeGfxVkGetInstance(gfxRuntime), &windowSurface);
        if (vkRes != VK_SUCCESS) {
            VK_END_PROGRAM
        }

        errC = createGfxSession(gfxRuntime, xrRuntime, settings.window.enableWSI, {windowSurface},
                                settings.graphics.gpu, settings.xr.forceXr, &gfxSession);
        if (errC) {
            ERRC_END_PROGRAM
        }
    }

    errC = foeGfxCreateUploadContext(gfxSession, &resUploader);
    if (errC) {
        VK_END_PROGRAM
    }

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
        vkRes = it.create(foeGfxVkGetDevice(gfxSession));
        if (vkRes != VK_SUCCESS) {
            VK_END_PROGRAM
        }
    }

    vkRes = renderPassPool.initialize(foeGfxVkGetDevice(gfxSession));
    if (vkRes != VK_SUCCESS)
        VK_END_PROGRAM

    vkRes = pipelinePool.initialize(gfxSession);
    if (vkRes != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    auto asyncTaskFunc = [&](std::function<void()> task) {
        asynchronousThreadPool.scheduleTask(std::move(task));
    };

    errC = shaderLoader.initialize(gfxSession, asyncTaskFunc);
    if (errC) {
        ERRC_END_PROGRAM
    }

    errC = fragDescriptorLoader.initialize(&fragmentDescriptorPool, &shaderLoader, &shaderPool,
                                           asyncTaskFunc);
    if (errC) {
        ERRC_END_PROGRAM
    }

    errC = imageLoader.initialize(gfxSession, asyncTaskFunc);
    if (errC) {
        ERRC_END_PROGRAM
    }

    errC = materialLoader.initialize(&fragDescriptorLoader, &fragDescriptorPool, &imageLoader,
                                     &imagePool, gfxSession, asyncTaskFunc);
    if (errC) {
        ERRC_END_PROGRAM
    }

    vkRes = cameraDescriptorPool.initialize(
        gfxSession,
        foeGfxVkGetBuiltinLayout(gfxSession,
                                 foeBuiltinDescriptorSetLayoutFlagBits::
                                     FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX),
        foeGfxVkGetBuiltinSetLayoutIndex(
            gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                            FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX));
    if (vkRes != VK_SUCCESS) {
        VK_END_PROGRAM
    }

    {
        auto shaderCode = loadShaderDataFromFile("data/shaders/simple/camera_tri.vert.spv");
        errC =
            foeGfxVkCreateShader(gfxSession,
                                 foeBuiltinDescriptorSetLayoutFlagBits::
                                     FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX,
                                 shaderCode.size(), reinterpret_cast<uint32_t *>(shaderCode.data()),
                                 VK_NULL_HANDLE, {}, &vertShader);

        // Vertex
        vertexDescriptor.mVertex = vertShader;
        vertexDescriptor.mVertexInputSCI = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };
        vertexDescriptor.mInputAssemblySCI = VkPipelineInputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        };

        // Fragment / Material
        foeShader *pFragShader = new foeShader{"theShader", &shaderLoader};
        if (!shaderPool.add(pFragShader)) {
            std::abort();
        }

        foeMaterial *theMaterial = new foeMaterial{"theMaterial", &materialLoader};
        materialPool.add(theMaterial);
        theMaterial->incrementUseCount();
        theMaterial->decrementUseCount();
        theMaterial = new foeMaterial{"theMaterial2", &materialLoader};
        materialPool.add(theMaterial);
        theMaterial->incrementUseCount();
        theMaterial->decrementUseCount();
    }

#ifdef FOE_XR_SUPPORT
    if (xrRuntime != FOE_NULL_HANDLE) {
        XrResult xrRes{XR_SUCCESS};

        errC = createXrSession(xrRuntime, gfxSession, &xrSession);
        if (errC) {
            ERRC_END_PROGRAM
        }

        // Session Views
        uint32_t viewConfigViewCount;
        xrRes =
            xrEnumerateViewConfigurationViews(foeXrOpenGetInstance(xrRuntime), xrSession.systemId,
                                              xrSession.type, 0, &viewConfigViewCount, nullptr);
        if (xrRes != XR_SUCCESS) {
            return xrRes;
        }

        std::vector<XrViewConfigurationView> viewConfigs;
        viewConfigs.resize(viewConfigViewCount);

        xrRes = xrEnumerateViewConfigurationViews(
            foeXrOpenGetInstance(xrRuntime), xrSession.systemId, xrSession.type, viewConfigs.size(),
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
                VkResult vkRes =
                    vkCreateImageView(foeGfxVkGetDevice(gfxSession), &viewCI, nullptr, &vkView);
                if (vkRes != VK_SUCCESS) {
                    return vkRes;
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
                    return vkRes;
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
            errC = foeXrOpenPollEvent(xrRuntime, event);
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
#endif

#ifdef FOE_XR_SUPPORT
    if (settings.xr.forceXr && xrSession.session == XR_NULL_HANDLE) {
        return -1;
    }
#endif

    return 0;
}

void Application::deinitialize() {
    if (gfxSession != FOE_NULL_HANDLE)
        vkDeviceWaitIdle(foeGfxVkGetDevice(gfxSession));

    { // Resource Unloading
        materialPool.unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            materialLoader.processUnloadRequests();
        }

        imagePool.unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            imageLoader.processUnloadRequests();
        }

        fragDescriptorPool.unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            fragDescriptorLoader.processUnloadRequests();
        }

        shaderPool.unloadAll();
        for (int i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES * 2; ++i) {
            shaderLoader.processUnloadRequests();
        }
    }

#ifdef FOE_XR_SUPPORT
    // OpenXR Cleanup
    if (xrSession.session != XR_NULL_HANDLE) {
        xrSession.requestExitSession();
        {
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
#endif

    // Resource Deinitialization
    materialLoader.deinitialize();
    imageLoader.deinitialize();
    fragDescriptorLoader.deinitialize();
    shaderLoader.deinitialize();

    for (auto &it : frameData) {
        it.destroy(foeGfxVkGetDevice(gfxSession));
    }

    for (auto &it : swapImageFramebuffers)
        vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);

    cameraDescriptorPool.deinitialize();

    pipelinePool.deinitialize();

    if (fragShader != FOE_NULL_HANDLE)
        foeGfxDestroyShader(gfxSession, fragShader);
    if (vertShader != FOE_NULL_HANDLE)
        foeGfxDestroyShader(gfxSession, vertShader);

    renderPassPool.deinitialize();

    swapchain.destroy(foeGfxVkGetDevice(gfxSession));
    if (windowSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(foeGfxVkGetInstance(gfxRuntime), windowSurface, nullptr);

    foeDestroyWindow();

    foeGfxDestroyUploadContext(resUploader);

#ifdef EDITOR_MODE
    imguiRenderer.deinitialize(gfxSession);
#endif

    if (gfxSession != FOE_NULL_HANDLE)
        foeGfxDestroySession(gfxSession);
    if (gfxRuntime != FOE_NULL_HANDLE)
        foeGfxDestroyRuntime(gfxRuntime);

#ifdef FOE_XR_SUPPORT
    if (xrRuntime != FOE_NULL_HANDLE)
        foeXrDestroyRuntime(xrRuntime);
#endif

    asynchronousThreadPool.terminate();
    synchronousThreadPool.terminate();

    // Output configuration settings to a YAML configuration file
    // saveSettings(settings);
}

namespace {

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

} // namespace

int Application::mainloop() {
    VkResult vkRes{VK_SUCCESS};

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
                        vkRes = vkGetPhysicalDeviceSurfaceFormatsKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), windowSurface, &formatCount,
                            nullptr);
                        if (vkRes != VK_SUCCESS)
                            VK_END_PROGRAM

                        std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats(
                            new VkSurfaceFormatKHR[formatCount]);

                        vkRes = vkGetPhysicalDeviceSurfaceFormatsKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), windowSurface, &formatCount,
                            surfaceFormats.get());
                        if (vkRes != VK_SUCCESS)
                            VK_END_PROGRAM

                        swapchain.surfaceFormat(surfaceFormats.get()[0]);
                    }

                    { // Present Modes
                        uint32_t modeCount;
                        vkGetPhysicalDeviceSurfacePresentModesKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), windowSurface, &modeCount,
                            nullptr);

                        std::unique_ptr<VkPresentModeKHR[]> presentModes(
                            new VkPresentModeKHR[modeCount]);

                        vkGetPhysicalDeviceSurfacePresentModesKHR(
                            foeGfxVkGetPhysicalDevice(gfxSession), windowSurface, &modeCount,
                            presentModes.get());

                        swapchain.presentMode(presentModes.get()[0]);
                    }
                }

                foeSwapchain newSwapchain;

                int width, height;
                foeWindowGetSize(&width, &height);
                vkRes = newSwapchain.create(foeGfxVkGetPhysicalDevice(gfxSession),
                                            foeGfxVkGetDevice(gfxSession), windowSurface,
                                            swapchain.surfaceFormat(), swapchain.presentMode(),
                                            swapchain, 3, width, height);
                if (vkRes != VK_SUCCESS)
                    VK_END_PROGRAM

                // If the old swapchain exists, we need to destroy it
                if (swapchain) {
                    swapchain.destroy(foeGfxVkGetDevice(gfxSession));
                }

                swapchain = newSwapchain;
                swapchainRebuilt = true;
            }

            // Acquire Target Presentation Images
            VkResult vkRes = swapchain.acquireNextImage(
                foeGfxVkGetDevice(gfxSession), frameData[nextFrameIndex].presentImageAcquired);
            if (vkRes == VK_TIMEOUT || vkRes == VK_NOT_READY) {
                // Waiting for an image to become ready
                goto SKIP_FRAME_RENDER;
            } else if (vkRes == VK_SUBOPTIMAL_KHR || vkRes == VK_ERROR_OUT_OF_DATE_KHR) {
                // Surface changed, best to rebuild swapchains
                goto SKIP_FRAME_RENDER;
            } else if (vkRes != VK_SUCCESS) {
                // Catastrophic error
                VK_END_PROGRAM
            }
            vkResetFences(foeGfxVkGetDevice(gfxSession), 1,
                          &frameData[nextFrameIndex].frameComplete);
            frameIndex = nextFrameIndex;

            // Resource Unload Requests
            shaderLoader.processUnloadRequests();
            fragDescriptorLoader.processUnloadRequests();
            imageLoader.processUnloadRequests();
            materialLoader.processUnloadRequests();

            // Resource Load Requests
            imageLoader.processLoadRequests();

            // Set camera descriptors
            bool camerasRemade = false;

            // Rendering
            vkResetCommandPool(foeGfxVkGetDevice(gfxSession), frameData[nextFrameIndex].commandPool,
                               0);

#ifdef FOE_XR_SUPPORT
            // OpenXR Render Section
            if (xrSession.session != XR_NULL_HANDLE) {
                XrResult xrRes{XR_SUCCESS};

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

                            vkRes = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                            if (vkRes != VK_SUCCESS)
                                VK_END_PROGRAM

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
                                        foeGfxVkFragmentDescriptor *pFragDescriptor;
                                        auto *theMaterial = materialPool.find("theMaterial2");

                                        if (theMaterial->getLoadState() !=
                                            foeResourceLoadState::Loaded)
                                            goto SKIP_XR_DRAW;

                                        pipelinePool.getPipeline(
                                            &vertexDescriptor,
                                            theMaterial->getGfxFragmentDescriptor(), xrRenderPass,
                                            0, &layout, &descriptorSetLayoutCount, &pipeline);

                                        vkCmdBindDescriptorSets(
                                            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                            0, 1, &it.camera.descriptor, 0, nullptr);

                                        if (auto set = theMaterial->getVkDescriptorSet(frameIndex);
                                            set != VK_NULL_HANDLE) {
                                            vkCmdBindDescriptorSets(
                                                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                layout, foeDescriptorSetLayoutIndex::FragmentShader,
                                                1, &set, 0, nullptr);
                                        }

                                        vkCmdBindPipeline(commandBuffer,
                                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                          pipeline);

                                        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

                                    SKIP_XR_DRAW:;
                                    }

                                    vkCmdEndRenderPass(commandBuffer);
                                }

                            vkRes = vkEndCommandBuffer(commandBuffer);
                            if (vkRes != VK_SUCCESS) {
                                FOE_LOG(General, Fatal, "Error with drawing: {}",
                                        std::error_code{vkRes}.message());
                            }

                            // Submission
                            VkPipelineStageFlags waitMask =
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
#endif

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
                    vkRes = vkCreateFramebuffer(foeGfxVkGetDevice(gfxSession), &framebufferCI,
                                                nullptr, &framebuffer);
                    if (vkRes != VK_SUCCESS)
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

                vkRes = vkBeginCommandBuffer(commandBuffer, &commandBufferBI);
                if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM
                }

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
                        foeGfxVkFragmentDescriptor *pFragDescriptor;
                        auto *theMaterial = materialPool.find("theMaterial2");

                        if (theMaterial->getLoadState() != foeResourceLoadState::Loaded)
                            goto SKIP_DRAW;

                        pipelinePool.getPipeline(
                            &vertexDescriptor, theMaterial->getGfxFragmentDescriptor(),
                            swapImageRenderPass, 0, &layout, &descriptorSetLayoutCount, &pipeline);

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                layout, 0, 1, &camera.descriptor, 0, nullptr);

                        if (auto set = theMaterial->getVkDescriptorSet(frameIndex);
                            set != VK_NULL_HANDLE) {
                            vkCmdBindDescriptorSets(
                                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                                foeDescriptorSetLayoutIndex::FragmentShader, 1, &set, 0, nullptr);
                        }

                        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                        vkCmdDraw(commandBuffer, 4, 1, 0, 0);

                    SKIP_DRAW:;
                    }

#ifdef EDITOR_MODE
                    { // ImGui
                        imguiRenderer.newFrame();
                        imguiState.runUI();
                        imguiRenderer.endFrame();

                        vkRes = imguiRenderer.update(foeGfxVkGetAllocator(gfxSession), frameIndex);
                        if (vkRes != VK_SUCCESS) {
                            VK_END_PROGRAM
                        }

                        imguiRenderer.draw(commandBuffer, frameIndex);
                    }
#endif

                    vkCmdEndRenderPass(commandBuffer);
                }

                vkRes = vkEndCommandBuffer(commandBuffer);
                if (vkRes != VK_SUCCESS) {
                    FOE_LOG(General, Fatal, "Error with drawing: {}",
                            std::error_code{vkRes}.message());
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

                auto queue = foeGfxGetQueue(getFirstQueue(gfxSession));
                VkResult vkRes = vkQueuePresentKHR(queue, &presentInfo);
                foeGfxReleaseQueue(getFirstQueue(gfxSession), queue);
                if (vkRes == VK_ERROR_OUT_OF_DATE_KHR) {
                    // The associated window has been resized, will be fixed for the next frame
                    vkRes = VK_SUCCESS;
                } else if (vkRes != VK_SUCCESS) {
                    VK_END_PROGRAM
                }
            }
        }
    SKIP_FRAME_RENDER:;

        synchronousThreadPool.waitForAllTasks();
    }
    FOE_LOG(General, Info, "Exiting main loop")

    return 0;
}