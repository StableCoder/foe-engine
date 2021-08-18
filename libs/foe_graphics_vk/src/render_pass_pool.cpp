/*
    Copyright (C) 2020-2021 George Cave.

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

#include <foe/graphics/vk/render_pass_pool.hpp>

#include <array>
#include <mutex>

#include "format.hpp"
#include "log.hpp"

VkResult foeGfxVkRenderPassPool::initialize(VkDevice device) noexcept {
    if (mDevice != VK_NULL_HANDLE)
        return VK_ERROR_INITIALIZATION_FAILED;

    mDevice = device;

    return VK_SUCCESS;
}

void foeGfxVkRenderPassPool::deinitialize() noexcept {
    std::scoped_lock lock{mSync};

    for (auto &it : mRenderPasses) {
        for (auto &variant : it.variants) {
            if (variant.renderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(mDevice, variant.renderPass, nullptr);
        }
    }

    mDevice = VK_NULL_HANDLE;
}

VkRenderPass foeGfxVkRenderPassPool::renderPass(
    std::vector<VkAttachmentDescription> const &attachments) {
    auto compatibleKeys = generateCompatibleKeys(attachments);
    auto variantKeys = generateVariantKeys(attachments);

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
    return generateRenderPass(compatibleKeys, variantKeys, attachments);
}

auto foeGfxVkRenderPassPool::renderPass(std::vector<VkFormat> const &formats,
                                   std::vector<VkSampleCountFlags> const &samples) -> VkRenderPass {
    std::vector<RenderPassCompatibleKey> compatibleKeys;
    compatibleKeys.reserve(formats.size());

    for (std::size_t idx = 0; idx < formats.size(); ++idx) {
        compatibleKeys.emplace_back(RenderPassCompatibleKey{
            .format = formats[idx],
            .samples = samples[idx],
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
    attachments.reserve(formats.size());
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
    auto variantKeys = generateVariantKeys(attachments);

    std::scoped_lock lock{mSync};
    renderPass = findRenderPass(compatibleKeys);
    if (renderPass != VK_NULL_HANDLE)
        return renderPass;
    return generateRenderPass(compatibleKeys, variantKeys, attachments);
}

auto foeGfxVkRenderPassPool::generateCompatibleKeys(
    std::vector<VkAttachmentDescription> const &attachments) const
    -> std::vector<RenderPassCompatibleKey> {
    std::vector<RenderPassCompatibleKey> key;
    key.reserve(attachments.size());

    for (auto const &it : attachments) {
        key.emplace_back(RenderPassCompatibleKey{
            .format = it.format,
            .samples = static_cast<VkSampleCountFlags>(it.samples),
        });
    }

    return key;
}

auto foeGfxVkRenderPassPool::generateVariantKeys(std::vector<VkAttachmentDescription> const &attachments)
    const -> std::vector<RenderPassVariantKey> {
    std::vector<RenderPassVariantKey> key;
    key.reserve(attachments.size());

    for (auto const &it : attachments) {
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

auto foeGfxVkRenderPassPool::findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey,
                                       std::vector<RenderPassVariantKey> const &variantKey)
    -> VkRenderPass {
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

auto foeGfxVkRenderPassPool::findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey)
    -> VkRenderPass {
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

auto foeGfxVkRenderPassPool::generateRenderPass(std::vector<RenderPassCompatibleKey> compatibleKey,
                                           std::vector<RenderPassVariantKey> variantKey,
                                           std::vector<VkAttachmentDescription> const &attachments)
    -> VkRenderPass {
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
            VkResult res = createRenderPass(attachments, &renderPass);
            if (res != VK_SUCCESS) {
                FOE_LOG(foeVkGraphics, Error, "Could not create a requested RenderPass")
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
    VkResult res = createRenderPass(attachments, &renderPass);
    if (res != VK_SUCCESS) {
        FOE_LOG(foeVkGraphics, Error, "Could not create a requested RenderPass")
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

auto foeGfxVkRenderPassPool::createRenderPass(std::vector<VkAttachmentDescription> const &attachments,
                                         VkRenderPass *pRenderPass) -> VkResult {
    constexpr auto cInvalidAttachment = UINT32_MAX;

    std::vector<VkAttachmentReference> colourReferences;
    colourReferences.reserve(attachments.size());

    VkAttachmentReference depthStencilReference{
        .attachment = cInvalidAttachment,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    for (uint32_t i = 0; i < attachments.size(); ++i) {
        auto &attachment = attachments[i];

        if (isDepthFormat(attachment.format)) {
            if (depthStencilReference.attachment != cInvalidAttachment) {
                FOE_LOG(foeVkGraphics, Error, "Two depth/stencil attachments requested on a Render Pass")
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
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = dependencies.size(),
        .pDependencies = dependencies.data(),
    };

    return vkCreateRenderPass(mDevice, &renderPassCI, nullptr, pRenderPass);
}