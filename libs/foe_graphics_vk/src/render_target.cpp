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

#include "render_target.hpp"

#include <foe/graphics/vk/image.hpp>
#include <vk_error_code.hpp>

#include "delayed_destructor.hpp"
#include "error_code.hpp"
#include "log.hpp"
#include "session.hpp"

namespace {

VkResult createTargetImage(foeGfxVkSession const *pGfxVkSession,
                           foeGfxVkRenderTargetSpec const &specification,
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
            .samples = static_cast<VkSampleCountFlagBits>(specification.samples),
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
            return vkRes;
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
                    .aspectMask = formatAspects(specification.format),
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkRes = vkCreateImageView(pGfxVkSession->device, &viewCI, nullptr, &data.view);
        if (vkRes != VK_SUCCESS)
            return vkRes;
    }

CREATE_FAILED:
    if (vkRes != VK_SUCCESS) {
        if (image.view != VK_NULL_HANDLE)
            vkDestroyImageView(pGfxVkSession->device, image.view, nullptr);

        if (image.image != VK_NULL_HANDLE)
            vmaDestroyImage(pGfxVkSession->allocator, image.image, image.alloc);
    } else {
        image = data;
    }

    return vkRes;
}

} // namespace

std::error_code foeGfxVkCreateRenderTarget(foeGfxSession session,
                                           foeGfxDelayedDestructor delayedDestructor,
                                           foeRenderPassPool *pRenderPassPool,
                                           foeGfxVkRenderTargetSpec const *pSpecifications,
                                           uint32_t count,
                                           foeGfxRenderTarget *pRenderTarget) {
    auto *pSession = session_from_handle(session);

    uint32_t imageCount = 0;
    std::vector<VkFormat> formats;
    std::vector<VkSampleCountFlags> samples;
    for (uint32_t i = 0; i < count; ++i) {
        imageCount += pSpecifications[i].count;
        formats.emplace_back(pSpecifications[i].format);
        samples.emplace_back(pSpecifications[i].samples);
    }

    VkRenderPass compatibleRenderPass = pRenderPassPool->renderPass(formats, samples);
    if (compatibleRenderPass == VK_NULL_HANDLE) {
        return FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_COULD_NOT_GET_COMPATIBLE_RENDER_PASS;
    }

    auto *pNewRenderTarget = new foeGfxVkRenderTarget{
        .pSession = pSession,
        .pDelayedDestructor = delayed_destructor_from_handle(delayedDestructor),
        .imageSpecifications =
            std::vector<foeGfxVkRenderTargetSpec>{pSpecifications, pSpecifications + count},
        .compatibleRenderPass = compatibleRenderPass,
        .images = std::vector<RenderTargetImageData>{imageCount, RenderTargetImageData{}},
        .indices = std::vector<uint8_t>(count, static_cast<uint8_t>(0)),
    };

    *pRenderTarget = render_target_to_handle(pNewRenderTarget);

    return VK_SUCCESS;
}

void foeGfxDestroyRenderTarget(foeGfxRenderTarget renderTarget) {
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
}

void foeGfxUpdateRenderTargetExtent(foeGfxRenderTarget renderTarget,
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

std::error_code foeGfxAcquireNextRenderTarget(foeGfxRenderTarget renderTarget,
                                              uint32_t maxBufferedFrames) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);
    uint32_t const numImages = pRenderTarget->imageSpecifications.size();
    std::error_code errC;

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
                // @todo Add delayed destruction for old images
                auto oldImage = image;
                foeGfxVkAddDelayedDestructionCall(
                    pRenderTarget->pDelayedDestructor,
                    [=](VkDevice device, VmaAllocator allocator) {
                        if (image.view != VK_NULL_HANDLE)
                            vkDestroyImageView(device, oldImage.view, nullptr);

                        if (image.image != VK_NULL_HANDLE)
                            vmaDestroyImage(allocator, oldImage.image, oldImage.alloc);
                    },
                    maxBufferedFrames);

                image = {};
            }

            errC = createTargetImage(pRenderTarget->pSession, spec, pRenderTarget->extent, image);
            if (errC) {
                FOE_LOG(foeVkGraphics, Error, "Failed to create RenderTarget Image: {} - {}",
                        errC.value(), errC.message());
                return errC;
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
        VkFramebuffer oldFramebuffer = pRenderTarget->framebuffer;
        foeGfxVkAddDelayedDestructionCall(
            pRenderTarget->pDelayedDestructor,
            [=](VkDevice device, VmaAllocator) {
                vkDestroyFramebuffer(device, oldFramebuffer, nullptr);
            },
            maxBufferedFrames);
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

    errC = vkCreateFramebuffer(pRenderTarget->pSession->device, &framebufferCI, nullptr,
                               &pRenderTarget->framebuffer);
    if (errC) {
        FOE_LOG(foeVkGraphics, Error, "Failed to create RenderTarget Framebuffer: {} - {}",
                errC.value(), errC.message());
        return errC;
    }

    return errC;
}

VkImage foeGfxVkGetRenderTargetImage(foeGfxRenderTarget renderTarget, uint32_t index) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    uint32_t offset = 0;
    for (uint32_t i = 0; i < index; ++i) {
        auto const &spec = pRenderTarget->imageSpecifications[i];
        auto &image = pRenderTarget->images[offset + pRenderTarget->indices[i]];

        offset += spec.count;
    }

    return pRenderTarget->images[offset + pRenderTarget->indices[index]].image;
}

VkFramebuffer foeGfxVkGetRenderTargetFramebuffer(foeGfxRenderTarget renderTarget) {
    auto *pRenderTarget = render_target_from_handle(renderTarget);

    return pRenderTarget->framebuffer;
}