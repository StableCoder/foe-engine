// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_PASS_POOL_HPP
#define RENDER_PASS_POOL_HPP

#include <foe/graphics/session.h>
#include <foe/graphics/vk/render_pass_pool.h>
#include <foe/handle.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <shared_mutex>
#include <vector>

class foeGfxVkRenderPassPoolImpl {
  public:
    foeResultSet initialize(foeGfxSession session) noexcept;
    void deinitialize() noexcept;

    auto renderPass(uint32_t attachmentCount,
                    VkAttachmentDescription const *pAttachments) -> VkRenderPass;

    auto renderPass(uint32_t attachmentCount,
                    VkFormat const *pFormats,
                    VkSampleCountFlags const *pSampleFlags) -> VkRenderPass;

  private:
    /// Elements that determine compatibility between render passes
    struct RenderPassCompatibleKey {
        VkFormat format;
        VkSampleCountFlags samples;

        bool operator==(RenderPassCompatibleKey const &rhs) const noexcept {
            return format == rhs.format && samples == rhs.samples;
        }
    };

    /// Variant-specific elements of a render pass
    struct RenderPassVariantKey {
        VkAttachmentLoadOp loadOp;
        VkAttachmentStoreOp storeOp;
        VkAttachmentLoadOp stencilLoadOp;
        VkAttachmentStoreOp stencilStoreOp;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;

        bool operator==(RenderPassVariantKey const &rhs) const noexcept {
            return loadOp == rhs.loadOp && storeOp == rhs.storeOp &&
                   stencilLoadOp == rhs.stencilLoadOp && stencilStoreOp == rhs.stencilStoreOp &&
                   initialLayout == rhs.initialLayout && finalLayout == rhs.finalLayout;
        }
    };

    struct RenderPassVariant {
        /// Variant keys for the render pass
        std::vector<RenderPassVariantKey> key;
        /// Render Pass handle
        VkRenderPass renderPass{VK_NULL_HANDLE};
    };

    struct RenderPassSet {
        /// Attachment formats/samples that define a set of compatible render passes
        std::vector<RenderPassCompatibleKey> key;
        /// The set of variants of compatible render passes
        std::vector<RenderPassVariant> variants;
    };

    auto generateCompatibleKeys(uint32_t attachmentCount,
                                VkAttachmentDescription const *pAttachments) const
        -> std::vector<RenderPassCompatibleKey>;
    auto generateVariantKeys(uint32_t attachmentCount, VkAttachmentDescription const *pAttachments)
        const -> std::vector<RenderPassVariantKey>;

    auto findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey,
                        std::vector<RenderPassVariantKey> const &variantKey) -> VkRenderPass;

    auto findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey) -> VkRenderPass;

    auto generateRenderPass(std::vector<RenderPassCompatibleKey> compatibleKey,
                            std::vector<RenderPassVariantKey> variantKey,
                            uint32_t attachmentCount,
                            VkAttachmentDescription const *pAttachments) -> VkRenderPass;

    auto createRenderPass(uint32_t attachmentCount,
                          VkAttachmentDescription const *pAttachments,
                          VkRenderPass *pRenderPass) -> VkResult;

    VkDevice mDevice{VK_NULL_HANDLE};

    std::shared_mutex mSync;

    std::vector<RenderPassSet> mRenderPasses;
};

FOE_DEFINE_HANDLE_CASTS(render_pass_pool, foeGfxVkRenderPassPoolImpl, foeGfxVkRenderPassPool)

#endif // RENDER_PASS_POOL_HPP