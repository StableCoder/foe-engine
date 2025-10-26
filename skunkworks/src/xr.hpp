// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_HPP
#define XR_HPP

#include <foe/graphics/delayed_caller.h>
#include <foe/graphics/render_target.h>
#include <foe/graphics/render_view_pool.h>
#include <foe/graphics/session.h>
#include <foe/result.h>
#include <foe/xr/openxr/camera_math.hpp>
#include <foe/xr/openxr/session.h>
#include <foe/xr/openxr/vk/vulkan.h>
#include <foe/xr/runtime.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct foeXrCamera {
    // Projection Data
    XrFovf fov;
    float nearZ, farZ;

    // View Data
    XrPosef pose;

    // Graphics Data
    VkDescriptorSet descriptor{VK_NULL_HANDLE};
};

inline glm::mat4 foeXrCameraProjectionMatrix(foeXrCamera const *pXrCamera) {
    return foeOpenXrProjectionMatrix(pXrCamera->fov, pXrCamera->nearZ, pXrCamera->farZ);
}

inline glm::mat4 foeXrCameraViewMatrix(foeXrCamera const *pXrCamera) {
    glm::mat4 rot = glm::mat4_cast(foeOpenXrPoseOrientation(pXrCamera->pose));
    glm::vec3 pos = glm::vec3{0.f, 0.f, -17.5f} + foeOpenXrPosePosition(pXrCamera->pose);

    glm::mat4 view = glm::translate(glm::mat4(1.f), pos) * rot;
    view = glm::inverse(view);

    return view;
}

struct foeXrVkSessionView {
    XrViewConfigurationView viewConfig;
    XrSwapchain swapchain;
    std::vector<XrSwapchainImageVulkanKHR> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
    foeXrCamera camera;

    VkSemaphore timelineSemaphore;
    foeGfxRenderTarget offscreenRenderTarget;
    foeGfxRenderView renderView{FOE_NULL_HANDLE};
};

struct BringupAppXrData {
    foeXrSession session{FOE_NULL_HANDLE};

    VkFormat colourFormat;
    VkFormat depthFormat{VK_FORMAT_D16_UNORM};
    VkRenderPass swapchainRenderPass;
    VkRenderPass offscreenRenderPass;

    std::vector<foeXrVkSessionView> views;

    XrFrameState frameState;
    foeGfxRenderViewPool renderViewPool{FOE_NULL_HANDLE};
};

foeResultSet createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime);

foeResultSet startXR(foeXrRuntime runtime,
                     foeGfxSession gfxSession,
                     foeGfxDelayedCaller gfxDelayedCaller,
                     VkFormat gfxDepthFormat,
                     VkSampleCountFlags gfxMSAA,
                     bool localPoll,
                     BringupAppXrData *pXrData);

foeResultSet stopXR(foeXrRuntime runtime,
                    foeGfxSession gfxSession,
                    bool localPoll,
                    BringupAppXrData *pXrData);

#endif // XR_HPP