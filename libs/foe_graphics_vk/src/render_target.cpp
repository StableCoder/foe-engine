// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_target.hpp"

#include <foe/graphics/vk/image.h>
#include <foe/graphics/vk/session.hpp>

#include "log.hpp"
#include "result.h"
#include "session.hpp"
#include "vk_result.h"

namespace {

foeResultSet createTargetImage(foeGfxVkSession const *pGfxVkSession,
                               foeGfxVkRenderTargetSpec const &specification,
                               VkSampleCountFlags samples,
                               VkExtent2D extent,
                               RenderTargetImageData &image) {
    RenderTargetImageData data{
        .latest = true,
    };
    VkResult vkRes{VK_SUCCESS};

    { // Image
        VkImageCreateInfo imageCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = specification.format,
            .extent =
                VkExtent3D{
                    .width = extent.width,
                    .height = extent.height,
                    .depth = 1,
                },
            .mipLevels = 1,
            .arrayLayers = 1U,
            .samples = static_cast<VkSampleCountFlagBits>(samples),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = specification.usage | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };

        vkRes = vmaCreateImage(pGfxVkSession->allocator, &imageCI, &allocCI, &data.image,
                               &data.alloc, nullptr);
        if (vkRes != VK_SUCCESS)
            return vk_to_foeResult(vkRes);
    }

    { // Image View
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = data.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = specification.format,
            .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                           VK_COMPONENT_SWIZZLE_A},
            .subresourceRange =
                VkImageSubresourceRange{
                    .aspectMask = foeGfxVkFormatAspects(specification.format),
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkRes = vkCreateImageView(pGfxVkSession->device, &viewCI, nullptr, &data.view);
        if (vkRes != VK_SUCCESS)
            return vk_to_foeResult(vkRes);
    }

    if (vkRes != VK_SUCCESS) {
        if (image.view != VK_NULL_HANDLE)
            vkDestroyImageView(pGfxVkSession->device, image.view, nullptr);

        if (image.image != VK_NULL_HANDLE)
            vmaDestroyImage(pGfxVkSession->allocator, image.image, image.alloc);
    } else {
        image = data;
    }

    return vk_to_foeResult(vkRes);
}

} // namespace

foeResultSet foeGfxVkCreateRenderTarget(foeGfxSession session,
                                        foeGfxDelayedCaller delayedCaller,
                                        foeGfxVkRenderTargetSpec const *pSpecifications,
                                        uint32_t count,
                                        VkSampleCountFlags samples,
                                        foeGfxRenderTarget *pRenderTarget) {
    auto *pSession = session_from_handle(session);

    uint32_t imageCount = 0;
    std::vector<VkFormat> formatList;
    std::vector<VkSampleCountFlags> sampleList;
    for (uint32_t i = 0; i < count; ++i) {
        imageCount += pSpecifications[i].count;
        formatList.emplace_back(pSpecifications[i].format);
        sampleList.emplace_back(samples);
    }

    VkRenderPass compatibleRenderPass =
        foeGfxVkGetRenderPassPool(session)->renderPass(formatList, sampleList);
    if (compatibleRenderPass == VK_NULL_HANDLE) {
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS);
    }

    auto *pNewRenderTarget = new foeGfxVkRenderTarget{
        .pSession = pSession,
        .delayedCaller = delayedCaller,
        .imageSpecifications =
            std::vector<foeGfxVkRenderTargetSpec>{pSpecifications, pSpecifications + count},
        .samples = samples,
        .compatibleRenderPass = compatibleRenderPass,
        .images = std::vector<RenderTargetImageData>{imageCount, RenderTargetImageData{}},
        .indices = std::vector<uint8_t>(count, static_cast<uint8_t>(0)),
    };

    *pRenderTarget = render_target_to_handle(pNewRenderTarget);

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

extern "C" void foeGfxDestroyRenderTarget(foeGfxRenderTarget renderTarget) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    // Framebuffer
    if (pRenderTarget->framebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(pRenderTarget->pSession->device, pRenderTarget->framebuffer, nullptr);

    for (auto const &image : pRenderTarget->images) {
        if (image.view != VK_NULL_HANDLE)
            vkDestroyImageView(pRenderTarget->pSession->device, image.view, nullptr);

        if (image.image != VK_NULL_HANDLE)
            vmaDestroyImage(pRenderTarget->pSession->allocator, image.image, image.alloc);
    }

    delete pRenderTarget;
}

extern "C" void foeGfxUpdateRenderTargetExtent(foeGfxRenderTarget renderTarget,
                                               uint32_t width,
                                               uint32_t height) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    if (pRenderTarget->extent.width == width && pRenderTarget->extent.height == height) {
        // Nothing to do here
        return;
    }

    pRenderTarget->extent = VkExtent2D{
        .width = width,
        .height = height,
    };

    for (auto &image : pRenderTarget->images) {
        image.latest = false;
    }
}

namespace {

void destroy_RenderTargetImageData(RenderTargetImageData *pOldImage, foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    if (pOldImage->view != VK_NULL_HANDLE)
        vkDestroyImageView(pSession->device, pOldImage->view, nullptr);

    if (pOldImage->image != VK_NULL_HANDLE)
        vmaDestroyImage(pSession->allocator, pOldImage->image, pOldImage->alloc);

    delete pOldImage;
}

void destroy_VkFramebuffer(VkFramebuffer framebuffer, foeGfxSession session) {
    auto *pSession = session_from_handle(session);

    vkDestroyFramebuffer(pSession->device, framebuffer, nullptr);
}

} // namespace

extern "C" foeResultSet foeGfxAcquireNextRenderTarget(foeGfxRenderTarget renderTarget,
                                                      uint32_t maxBufferedFrames) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);
    uint32_t const numImages = static_cast<uint32_t>(pRenderTarget->imageSpecifications.size());
    foeResultSet result = to_foeResult(FOE_GRAPHICS_VK_SUCCESS);

    // Increment the indices, making sure to not go over the specified image counts for each type
    for (uint32_t i = 0; i < numImages; ++i) {
        ++pRenderTarget->indices[i];
        if (pRenderTarget->indices[i] >= pRenderTarget->imageSpecifications[i].count) {
            pRenderTarget->indices[i] = 0;
        }
    }

    // Make sure the images are all valid. This *could* be merged with the above loop, however for
    // ease of reading, it's separated for now
    std::vector<VkImageView> views;
    views.reserve(numImages);

    uint32_t offset = 0;
    for (uint32_t i = 0; i < numImages; ++i) {
        auto const &spec = pRenderTarget->imageSpecifications[i];
        auto &image = pRenderTarget->images[offset + pRenderTarget->indices[i]];

        if (!image.latest) {
            // Need to re-create the image, maybe delete the old
            if (image.image != VK_NULL_HANDLE) {
                RenderTargetImageData const *pOldImage = new RenderTargetImageData{image};

                foeGfxAddDelayedCall(pRenderTarget->delayedCaller,
                                     (PFN_foeGfxDelayedCall)destroy_RenderTargetImageData,
                                     (void *)pOldImage, maxBufferedFrames);

                image = {};
            }

            result = createTargetImage(pRenderTarget->pSession, spec, pRenderTarget->samples,
                                       pRenderTarget->extent, image);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeVkGraphics, Error, "Failed to create RenderTarget Image: {}", buffer);

                return result;
            }

            image.latest = true;
        }

        // Get the view for the framebuffer creation
        views.emplace_back(image.view);

        // Update the image offset
        offset += spec.count;
    }

    // Destroy old framebuffer in a delayed manner
    if (pRenderTarget->framebuffer != VK_NULL_HANDLE) {
        foeGfxAddDelayedCall(pRenderTarget->delayedCaller,
                             (PFN_foeGfxDelayedCall)destroy_VkFramebuffer,
                             (void *)pRenderTarget->framebuffer, maxBufferedFrames);
        pRenderTarget->framebuffer = VK_NULL_HANDLE;
    }

    // Create new framebuffer
    VkFramebufferCreateInfo framebufferCI{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = pRenderTarget->compatibleRenderPass,
        .attachmentCount = static_cast<uint32_t>(views.size()),
        .pAttachments = views.data(),
        .width = pRenderTarget->extent.width,
        .height = pRenderTarget->extent.height,
        .layers = 1,
    };

    result = vk_to_foeResult(vkCreateFramebuffer(pRenderTarget->pSession->device, &framebufferCI,
                                                 nullptr, &pRenderTarget->framebuffer));
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeVkGraphics, Error, "Failed to create RenderTarget Framebuffer: {}", buffer);

        return result;
    }

    return result;
}

auto foeGfxVkGetRenderTargetSamples(foeGfxRenderTarget renderTarget) -> VkSampleCountFlags {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    return pRenderTarget->samples;
}

auto foeGfxVkGetRenderTargetImage(foeGfxRenderTarget renderTarget, uint32_t index) -> VkImage {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    uint32_t offset = 0;
    for (uint32_t i = 0; i < index; ++i) {
        auto const &spec = pRenderTarget->imageSpecifications[i];

        offset += spec.count;
    }

    return pRenderTarget->images[offset + pRenderTarget->indices[index]].image;
}

auto foeGfxVkGetRenderTargetImageView(foeGfxRenderTarget renderTarget, uint32_t index)
    -> VkImageView {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    uint32_t offset = 0;
    for (uint32_t i = 0; i < index; ++i) {
        auto const &spec = pRenderTarget->imageSpecifications[i];

        offset += spec.count;
    }

    return pRenderTarget->images[offset + pRenderTarget->indices[index]].view;
}

auto foeGfxVkGetRenderTargetFramebuffer(foeGfxRenderTarget renderTarget) -> VkFramebuffer {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    return pRenderTarget->framebuffer;
}