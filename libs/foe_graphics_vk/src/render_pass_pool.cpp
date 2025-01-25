// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/render_pass_pool.h>

#include <foe/graphics/vk/format.h>

#include <array>
#include <mutex>

#include "log.hpp"
#include "render_pass_pool.hpp"
#include "session.hpp"
#include "vk_result.h"

foeResultSet foeGfxVkRenderPassPoolImpl::initialize(foeGfxSession session) noexcept {
    auto *pSession = session_from_handle(session);

    mDevice = pSession->device;

    return vk_to_foeResult(VK_SUCCESS);
}

void foeGfxVkRenderPassPoolImpl::deinitialize() noexcept {
    std::scoped_lock lock{mSync};

    for (auto &it : mRenderPasses) {
        for (auto &variant : it.variants) {
            if (variant.renderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(mDevice, variant.renderPass, nullptr);
        }
    }

    mDevice = VK_NULL_HANDLE;
}

VkRenderPass foeGfxVkRenderPassPoolImpl::renderPass(uint32_t attachmentCount,
                                                    VkAttachmentDescription const *pAttachments) {
    auto compatibleKeys = generateCompatibleKeys(attachmentCount, pAttachments);
    auto variantKeys = generateVariantKeys(attachmentCount, pAttachments);

    // Try to get the renderpass in shared read mode
    std::shared_lock sharedLock{mSync};
    VkRenderPass renderPass = findRenderPass(compatibleKeys, variantKeys);
    if (renderPass != VK_NULL_HANDLE)
        return renderPass;
    sharedLock.unlock();

    // Find/create the render pass in exclusive mode
    std::scoped_lock lock{mSync};
    renderPass = findRenderPass(compatibleKeys, variantKeys);
    if (renderPass != VK_NULL_HANDLE)
        return renderPass;
    return generateRenderPass(compatibleKeys, variantKeys, attachmentCount, pAttachments);
}

auto foeGfxVkRenderPassPoolImpl::renderPass(uint32_t attachmentCount,
                                            VkFormat const *pFormats,
                                            VkSampleCountFlags const *pSampleFlags)
    -> VkRenderPass {
    std::vector<RenderPassCompatibleKey> compatibleKeys;
    compatibleKeys.reserve(attachmentCount);

    for (std::size_t idx = 0; idx < attachmentCount; ++idx) {
        compatibleKeys.emplace_back(RenderPassCompatibleKey{
            .format = pFormats[idx],
            .samples = pSampleFlags[idx],
        });
    }

    // Try to get the renderpass in shared read mode
    std::shared_lock sharedLock{mSync};
    VkRenderPass renderPass = findRenderPass(compatibleKeys);
    if (renderPass != VK_NULL_HANDLE)
        return renderPass;
    sharedLock.unlock();

    // If no variant yet exists, create a basic one now

    // Generate attachment descriptions/variant keys
    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(attachmentCount);

    for (auto const &key : compatibleKeys) {
        attachments.emplace_back(VkAttachmentDescription{
            .flags = 0,
            .format = key.format,
            .samples = static_cast<VkSampleCountFlagBits>(key.samples),
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }
    auto variantKeys = generateVariantKeys(attachmentCount, attachments.data());

    std::scoped_lock lock{mSync};
    renderPass = findRenderPass(compatibleKeys);
    if (renderPass != VK_NULL_HANDLE)
        return renderPass;
    return generateRenderPass(compatibleKeys, variantKeys, attachmentCount, attachments.data());
}

auto foeGfxVkRenderPassPoolImpl::generateCompatibleKeys(
    uint32_t attachmentCount, VkAttachmentDescription const *pAttachments) const
    -> std::vector<RenderPassCompatibleKey> {
    std::vector<RenderPassCompatibleKey> key;
    key.reserve(attachmentCount);

    for (uint32_t i = 0; i < attachmentCount; ++i) {
        auto &it = pAttachments[i];

        key.emplace_back(RenderPassCompatibleKey{
            .format = it.format,
            .samples = static_cast<VkSampleCountFlags>(it.samples),
        });
    }

    return key;
}

auto foeGfxVkRenderPassPoolImpl::generateVariantKeys(
    uint32_t attachmentCount, VkAttachmentDescription const *pAttachments) const
    -> std::vector<RenderPassVariantKey> {
    std::vector<RenderPassVariantKey> key;
    key.reserve(attachmentCount);

    for (uint32_t i = 0; i < attachmentCount; ++i) {
        auto &it = pAttachments[i];

        key.emplace_back(RenderPassVariantKey{
            .loadOp = it.loadOp,
            .storeOp = it.storeOp,
            .stencilLoadOp = it.stencilLoadOp,
            .stencilStoreOp = it.stencilStoreOp,
            .initialLayout = it.initialLayout,
            .finalLayout = it.finalLayout,
        });
    }

    return key;
}

auto foeGfxVkRenderPassPoolImpl::findRenderPass(
    std::vector<RenderPassCompatibleKey> const &compatibleKey,
    std::vector<RenderPassVariantKey> const &variantKey) -> VkRenderPass {
    for (auto &it : mRenderPasses) {
        if (it.key == compatibleKey) {
            // Have the same compatible elements, now need to find the specific variant
            for (auto &variant : it.variants) {
                if (variant.key == variantKey) {
                    // Found it
                    return variant.renderPass;
                }
            }

            return VK_NULL_HANDLE;
        }
    }

    return VK_NULL_HANDLE;
}

auto foeGfxVkRenderPassPoolImpl::findRenderPass(
    std::vector<RenderPassCompatibleKey> const &compatibleKey) -> VkRenderPass {
    for (auto &it : mRenderPasses) {
        if (it.key == compatibleKey) {
            for (auto &variant : it.variants) {
                return variant.renderPass;
            }

            return VK_NULL_HANDLE;
        }
    }

    return VK_NULL_HANDLE;
}

auto foeGfxVkRenderPassPoolImpl::generateRenderPass(
    std::vector<RenderPassCompatibleKey> compatibleKey,
    std::vector<RenderPassVariantKey> variantKey,
    uint32_t attachmentCount,
    VkAttachmentDescription const *pAttachments) -> VkRenderPass {
    for (auto &it : mRenderPasses) {
        if (it.key == compatibleKey) {
            // Have the same compatible elements, now need to find the specific variant
            for (auto &variant : it.variants) {
                if (variant.key == variantKey) {
                    // Found it
                    return variant.renderPass;
                }
            }

            // Create the specific variant
            VkRenderPass renderPass{VK_NULL_HANDLE};
            VkResult res = createRenderPass(attachmentCount, pAttachments, &renderPass);
            if (res != VK_SUCCESS) {
                FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                        "Could not create a requested RenderPass")
                return VK_NULL_HANDLE;
            }

            it.variants.emplace_back(RenderPassVariant{
                .key = variantKey,
                .renderPass = renderPass,
            });

            return renderPass;
        }
    }

    // Create both a new compatible set and variant
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkResult res = createRenderPass(attachmentCount, pAttachments, &renderPass);
    if (res != VK_SUCCESS) {
        FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR, "Could not create a requested RenderPass")
        return VK_NULL_HANDLE;
    }

    RenderPassSet newEntry = {
        .key = compatibleKey,
        .variants =
            {
                RenderPassVariant{
                    .key = variantKey,
                    .renderPass = renderPass,
                },
            },
    };

    mRenderPasses.emplace_back(newEntry);

    return renderPass;
}

auto foeGfxVkRenderPassPoolImpl::createRenderPass(uint32_t attachmentCount,
                                                  VkAttachmentDescription const *pAttachments,
                                                  VkRenderPass *pRenderPass) -> VkResult {
    constexpr auto cInvalidAttachment = UINT32_MAX;

    std::vector<VkAttachmentReference> colourReferences;
    colourReferences.reserve(attachmentCount);

    VkAttachmentReference depthStencilReference{
        .attachment = cInvalidAttachment,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    for (uint32_t i = 0; i < attachmentCount; ++i) {
        auto &attachment = pAttachments[i];

        if (foeGfxVkIsDepthFormat(attachment.format)) {
            if (depthStencilReference.attachment != cInvalidAttachment) {
                FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                        "Two depth/stencil attachments requested on a Render Pass")
                return VK_ERROR_INITIALIZATION_FAILED;
            }
            depthStencilReference = VkAttachmentReference{
                .attachment = i,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };
        } else {
            colourReferences.emplace_back(VkAttachmentReference{
                .attachment = i,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            });
        }
    }

    VkSubpassDescription subpassDescription{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colourReferences.size()),
        .pColorAttachments = colourReferences.data(),
        .pDepthStencilAttachment = (depthStencilReference.attachment != cInvalidAttachment)
                                       ? &depthStencilReference
                                       : nullptr,
    };

    std::array<VkSubpassDependency, 2> dependencies{
        VkSubpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        },
        VkSubpassDependency{
            .srcSubpass = 0,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .srcAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        },
    };

    VkRenderPassCreateInfo renderPassCI{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachmentCount,
        .pAttachments = pAttachments,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = dependencies.size(),
        .pDependencies = dependencies.data(),
    };

    return vkCreateRenderPass(mDevice, &renderPassCI, nullptr, pRenderPass);
}