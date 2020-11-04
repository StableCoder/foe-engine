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

#ifndef FOE_GRAPHICS_RENDER_PASS_HPP
#define FOE_GRAPHICS_RENDER_PASS_HPP

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <shared_mutex>
#include <vector>

class foeRenderPassPool {
  public:
    FOE_GFX_EXPORT VkResult initialize(VkDevice device) noexcept;
    FOE_GFX_EXPORT void deinitialize() noexcept;

    /// Returns a specific render pass for the given attachment
    FOE_GFX_EXPORT auto renderPass(std::vector<VkAttachmentDescription> const &attachments)
        -> VkRenderPass;

  private:
    /// Elements that determine compatability between render passes
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

    auto generateCompatibleKeys(std::vector<VkAttachmentDescription> const &attachments) const
        -> std::vector<RenderPassCompatibleKey>;
    auto generateVariantKeys(std::vector<VkAttachmentDescription> const &attachments) const
        -> std::vector<RenderPassVariantKey>;

    auto findRenderPass(std::vector<RenderPassCompatibleKey> compatibleKey,
                        std::vector<RenderPassVariantKey> variantKey,
                        std::vector<VkAttachmentDescription> const &attachments) -> VkRenderPass;

    auto generateRenderPass(std::vector<RenderPassCompatibleKey> compatibleKey,
                            std::vector<RenderPassVariantKey> variantKey,
                            std::vector<VkAttachmentDescription> const &attachments)
        -> VkRenderPass;

    auto createRenderPass(std::vector<VkAttachmentDescription> const &attachments,
                          VkRenderPass *pRenderPass) -> VkResult;

    VkDevice mDevice{VK_NULL_HANDLE};

    std::shared_mutex mSync;

    std::vector<RenderPassSet> mRenderPasses;
};

#endif // FOE_GRAPHICS_RENDER_PASS_HPP