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

    /**
     * @brief Returns a specific render pass for the given attachment
     * @param attachments Describes the attachment set the render pass is defined by
     * @return A valid render pass handle, VK_NULL_HANDLE otherwise
     *
     * If the render pass described already exists, this will return the handle to that. If it
     * doesn't it will be generated.
     */
    FOE_GFX_EXPORT auto renderPass(std::vector<VkAttachmentDescription> const &attachments)
        -> VkRenderPass;

    /**
     * @brief Returns a compatible render pass for the given formats/samples
     * @param formats Image formats corresponding to the attached images for the render pass
     * @param samples Sample counts corresponding to the attached images for the render pass
     * @return First compatible render pass handle, VK_NULL_HANDLE otherwise
     * @note This is intended for finding render passes quickly for use in geneating framebuffers
     * rather than rendering.
     *
     * Thanks to the compatability of render passes based on a minimal set of the formnats and
     * sample count, this function will attempt to find *any* compatable render passes and return
     * the first found.
     *
     * If no render pass is found, it will generate a basic one instead.
     */
    FOE_GFX_EXPORT auto renderPass(std::vector<VkFormat> const &formats,
                                   std::vector<VkSampleCountFlags> const &samples) -> VkRenderPass;

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

    auto findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey,
                        std::vector<RenderPassVariantKey> const &variantKey) -> VkRenderPass;

    auto findRenderPass(std::vector<RenderPassCompatibleKey> const &compatibleKey) -> VkRenderPass;

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