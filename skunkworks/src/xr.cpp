// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "xr.hpp"

#include <foe/graphics/backend.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/render_target.h>
#include <foe/graphics/vk/session.h>
#include <foe/xr/openxr/runtime.h>
#include <foe/xr/openxr/vk/vulkan.h>

#include "log.hpp"
#include "result.h"
#include "vk_result.h"
#include "xr_result.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

foeResultSet createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime) {
    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    if (strcmp(foeGfxBackendName(), "Vulkan") == 0) {
        extensions.emplace_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
    } else {
        std::abort();
    }

    return foeOpenXrCreateRuntime("FoE Engine", 0, layers.size(), layers.data(), extensions.size(),
                                  extensions.data(), false, debugLogging, pRuntime);
}

namespace {

foeResultSet createXrSession(foeXrRuntime runtime,
                             foeGfxSession gfxSession,
                             foeXrSession *pSession) {
    foeResultSet result;
    XrSystemId xrSystemId{};

    // OpenXR SystemId
    if (foeOpenXrGetInstance(runtime) != XR_NULL_HANDLE) {
        XrSystemGetInfo systemGetInfo{
            .type = XR_TYPE_SYSTEM_GET_INFO,
            .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
        };

        result = xr_to_foeResult(
            xrGetSystem(foeOpenXrGetInstance(runtime), &systemGetInfo, &xrSystemId));
        if (result.value != FOE_SUCCESS) {
            return result;
        }
    }

    // Types
    uint32_t viewConfigCount;
    result = xr_to_foeResult(xrEnumerateViewConfigurations(
        foeOpenXrGetInstance(runtime), xrSystemId, 0, &viewConfigCount, nullptr));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    std::vector<XrViewConfigurationType> xrViewConfigTypes;
    xrViewConfigTypes.resize(viewConfigCount);

    result = xr_to_foeResult(xrEnumerateViewConfigurations(
        foeOpenXrGetInstance(runtime), xrSystemId, xrViewConfigTypes.size(), &viewConfigCount,
        xrViewConfigTypes.data()));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // Is View Mutable??
    XrViewConfigurationProperties xrViewConfigProps{.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
    result = xr_to_foeResult(xrGetViewConfigurationProperties(
        foeOpenXrGetInstance(runtime), xrSystemId, xrViewConfigTypes[0], &xrViewConfigProps));
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // Check graphics requirements
    XrGraphicsRequirementsVulkanKHR gfxRequirements;
    result = foeXrGetVulkanGraphicsRequirements(foeOpenXrGetInstance(runtime), xrSystemId,
                                                &gfxRequirements);
    if (result.value != FOE_SUCCESS) {
        return result;
    }

    // XrSession
    XrGraphicsBindingVulkanKHR gfxBinding{
        .type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR,
        .instance = foeGfxVkGetInstance(gfxSession),
        .physicalDevice = foeGfxVkGetPhysicalDevice(gfxSession),
        .device = foeGfxVkGetDevice(gfxSession),
        .queueFamilyIndex = 0,
        .queueIndex = 0,
    };

    return foeOpenXrCreateSession(runtime, xrSystemId, xrViewConfigTypes[0], &gfxBinding, pSession);
}

} // namespace

foeResultSet startXR(foeXrRuntime runtime,
                     foeGfxSession gfxSession,
                     foeGfxDelayedCaller gfxDelayedCaller,
                     VkFormat gfxDepthFormat,
                     VkSampleCountFlags gfxMSAA,
                     bool localPoll,
                     BringupAppXrData *pXrData) {
    foeResultSet result = to_foeResult(FOE_SKUNKWORKS_SUCCESS);

    if (runtime == FOE_NULL_HANDLE) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "Tried to start an XR session, but no XR runtime has been started");
    } else {
        result = createXrSession(runtime, gfxSession, &pXrData->session);
        if (result.value != FOE_SUCCESS)
            goto START_XR_FAILED;

        // OpenXR Session Begin

        // Wait for the session to be ready
        while (foeOpenXrGetSessionState(pXrData->session) != XR_SESSION_STATE_READY) {
            if (localPoll) {
                result = foeXrProcessEvents(runtime);
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

        { // Swapchain Format
            uint32_t formatCount;
            std::vector<int64_t> swapchainFormats;
            result = foeOpenXrEnumerateSwapchainFormats(pXrData->session, &formatCount, nullptr);
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            swapchainFormats.resize(formatCount);
            result = foeOpenXrEnumerateSwapchainFormats(pXrData->session, &formatCount,
                                                        swapchainFormats.data());
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            pXrData->colourFormat = static_cast<VkFormat>(swapchainFormats[0]);
            pXrData->depthFormat = gfxDepthFormat;
        }

        { // Session Views
            uint32_t viewConfigViewCount;
            result = xr_to_foeResult(xrEnumerateViewConfigurationViews(
                foeOpenXrGetInstance(runtime), foeOpenXrGetSystemId(pXrData->session),
                foeOpenXrGetViewConfigurationType(pXrData->session), 0, &viewConfigViewCount,
                nullptr));
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            std::vector<XrViewConfigurationView> viewConfigs{
                viewConfigViewCount, XrViewConfigurationView{
                                         .type = XR_TYPE_VIEW_CONFIGURATION_VIEW,
                                     }};

            result = xr_to_foeResult(xrEnumerateViewConfigurationViews(
                foeOpenXrGetInstance(runtime), foeOpenXrGetSystemId(pXrData->session),
                foeOpenXrGetViewConfigurationType(pXrData->session), viewConfigs.size(),
                &viewConfigViewCount, viewConfigs.data()));
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            pXrData->views.clear();
            for (auto const &it : viewConfigs) {
                pXrData->views.emplace_back(foeXrVkSessionView{.viewConfig = it});
            }
        }

        { // Compatible render passes
            foeGfxVkRenderPassPool renderPassPool = foeGfxVkGetRenderPassPool(gfxSession);

            VkAttachmentDescription xrAttachmentDescription{
                .format = pXrData->colourFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            };

            pXrData->swapchainRenderPass =
                foeGfxVkGetRenderPass(renderPassPool, 1, &xrAttachmentDescription);

            std::array<VkAttachmentDescription, 2> xrOffscreenAttachmentDescription{
                VkAttachmentDescription{
                    .format = pXrData->colourFormat,
                    .samples = static_cast<VkSampleCountFlagBits>(gfxMSAA),
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                },
                VkAttachmentDescription{
                    .format = pXrData->depthFormat,
                    .samples = static_cast<VkSampleCountFlagBits>(gfxMSAA),
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }};

            pXrData->offscreenRenderPass =
                foeGfxVkGetRenderPass(renderPassPool, xrOffscreenAttachmentDescription.size(),
                                      xrOffscreenAttachmentDescription.data());
        }

        // Render View Pool
        result =
            foeGfxCreateRenderViewPool(gfxSession, pXrData->views.size(), &pXrData->renderViewPool);
        if (result.value != FOE_SUCCESS)
            goto START_XR_FAILED;

        for (auto &view : pXrData->views) {
            // Offscreen Render Targets
            std::array<foeGfxVkRenderTargetSpec, 2> offscreenSpecs = {
                foeGfxVkRenderTargetSpec{
                    .format = pXrData->colourFormat,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .count = 3,
                },
                foeGfxVkRenderTargetSpec{
                    .format = pXrData->depthFormat,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .count = 3,
                },
            };

            result = foeGfxVkCreateRenderTarget(gfxSession, gfxDelayedCaller, offscreenSpecs.data(),
                                                offscreenSpecs.size(), gfxMSAA,
                                                &view.offscreenRenderTarget);
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            foeGfxUpdateRenderTargetExtent(view.offscreenRenderTarget,
                                           view.viewConfig.recommendedImageRectWidth,
                                           view.viewConfig.recommendedImageRectHeight);

            // Swapchain
            XrSwapchainCreateInfo swapchainCI{
                .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
                .createFlags = 0,
                .usageFlags =
                    XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT,
                .format = pXrData->colourFormat,
                .sampleCount = 1,
                .width = view.viewConfig.recommendedImageRectWidth,
                .height = view.viewConfig.recommendedImageRectHeight,
                .faceCount = 1,
                .arraySize = 1,
                .mipCount = 1,
            };

            result = xr_to_foeResult(xrCreateSwapchain(foeOpenXrGetSession(pXrData->session),
                                                       &swapchainCI, &view.swapchain));
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            // Images
            uint32_t imageCount = view.images.size();
            result = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, &imageCount, nullptr);
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            view.images.resize(imageCount);
            result = foeOpenXrEnumerateSwapchainVkImages(view.swapchain, &imageCount,
                                                         view.images.data());
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;

            // Image Views
            VkImageViewCreateInfo viewCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = pXrData->colourFormat,
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
                if (result.value != FOE_SUCCESS)
                    goto START_XR_FAILED;

                view.imageViews.emplace_back(vkView);
            }

            // VkFramebuffer
            for (auto it : view.imageViews) {
                VkFramebufferCreateInfo framebufferCI{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = pXrData->swapchainRenderPass,
                    .attachmentCount = 1,
                    .pAttachments = &it,
                    .width = view.viewConfig.recommendedImageRectWidth,
                    .height = view.viewConfig.recommendedImageRectHeight,
                    .layers = 1,
                };

                VkFramebuffer newFramebuffer;
                result = vk_to_foeResult(vkCreateFramebuffer(
                    foeGfxVkGetDevice(gfxSession), &framebufferCI, nullptr, &newFramebuffer));
                if (result.value != FOE_SUCCESS)
                    goto START_XR_FAILED;

                view.framebuffers.emplace_back(newFramebuffer);
            }

            result = foeGfxAllocateRenderView(pXrData->renderViewPool, &view.renderView);
            if (result.value != FOE_SUCCESS)
                goto START_XR_FAILED;
        }

        result = foeOpenXrBeginSession(pXrData->session);
        if (result.value != FOE_SUCCESS)
            goto START_XR_FAILED;

        FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Started new XR session {}",
                static_cast<void *>(foeOpenXrGetSession(pXrData->session)));
    }

START_XR_FAILED:
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Failed to start XR with error {}", buffer);

        // stopXR(localPoll);
    }

    return result;
}

foeResultSet stopXR(foeXrRuntime runtime,
                    foeGfxSession gfxSession,
                    bool localPoll,
                    BringupAppXrData *pXrData) {
    foeResultSet result = to_foeResult(FOE_SKUNKWORKS_SUCCESS);

    if (pXrData->session != FOE_NULL_HANDLE) {
        foeOpenXrRequestExitSession(pXrData->session);

        while (foeOpenXrGetSessionState(pXrData->session) != XR_SESSION_STATE_STOPPING) {
            if (localPoll) {
                result = foeXrProcessEvents(runtime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL,
                            "Failed to stop XR at {}:{} with error {}", __FILE__, __LINE__, buffer);
                    return result;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        foeOpenXrEndSession(pXrData->session);

        while (foeOpenXrGetSessionState(pXrData->session) != XR_SESSION_STATE_IDLE) {
            if (localPoll) {
                result = foeXrProcessEvents(runtime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL,
                            "Failed to stop XR at {}:{} with error {}", __FILE__, __LINE__, buffer);
                    return result;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        for (auto &view : pXrData->views) {
            if (view.offscreenRenderTarget != FOE_NULL_HANDLE)
                foeGfxDestroyRenderTarget(view.offscreenRenderTarget);

            for (auto it : view.framebuffers)
                vkDestroyFramebuffer(foeGfxVkGetDevice(gfxSession), it, nullptr);

            for (auto it : view.imageViews)
                vkDestroyImageView(foeGfxVkGetDevice(gfxSession), it, nullptr);

            if (view.swapchain != XR_NULL_HANDLE)
                xrDestroySwapchain(view.swapchain);
        }

        if (pXrData->renderViewPool != FOE_NULL_HANDLE)
            foeGfxDestroyRenderViewPool(gfxSession, pXrData->renderViewPool);
        pXrData->renderViewPool = FOE_NULL_HANDLE;

        while (foeOpenXrGetSessionState(pXrData->session) != XR_SESSION_STATE_EXITING) {
            if (localPoll) {
                result = foeXrProcessEvents(runtime);
                if (result.value != FOE_SUCCESS) {
                    char buffer[FOE_MAX_RESULT_STRING_SIZE];
                    result.toString(result.value, buffer);
                    FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL,
                            "Failed to stop XR at {}:{} with error {}", __FILE__, __LINE__, buffer);
                    return result;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        foeXrDestroySession(pXrData->session);
        pXrData->session = FOE_NULL_HANDLE;
    }

    return result;
}