// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_PIPELINE_POOL_HPP
#define FOE_GRAPHICS_VK_PIPELINE_POOL_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

struct foeGfxVertexDescriptor;
struct foeGfxVkFragmentDescriptor;
class foeGfxVkBuiltinDescriptorSets;

class foeGfxVkPipelinePool {
  public:
    FOE_GFX_EXPORT foeResultSet initialize(foeGfxSession session) noexcept;
    FOE_GFX_EXPORT void deinitialize() noexcept;
    FOE_GFX_EXPORT bool initialized() const noexcept;

    FOE_GFX_EXPORT foeResultSet getPipeline(foeGfxVertexDescriptor *vertexDescriptor,
                                            foeGfxVkFragmentDescriptor *fragmentDescriptor,
                                            VkRenderPass renderPass,
                                            uint32_t subpass,
                                            VkSampleCountFlags samples,
                                            VkPipelineLayout *pPipelineLayout,
                                            uint32_t *pDescriptorSetLayoutCount,
                                            VkPipeline *pPipeline);

  private:
    // Counts upto 64, powers of 2.
    static constexpr size_t cMaxSampleOptions = 7;

    VkResult createPipeline(foeGfxVertexDescriptor *vertexDescriptor,
                            foeGfxVkFragmentDescriptor *fragmentDescriptor,
                            VkRenderPass renderPass,
                            uint32_t subpass,
                            VkSampleCountFlags samples,
                            VkPipelineLayout *pPipelineLayout,
                            uint32_t *pDescriptorSetLayoutCount,
                            VkPipeline *pPipeline) const noexcept;

    struct PipelineSet {
        // Key
        foeGfxVertexDescriptor *vertexDescriptor;
        foeGfxVkFragmentDescriptor *fragmentDescriptor;

        VkRenderPass renderPass;
        uint32_t subpass;

        // Locally Managed
        VkPipelineLayout layout;
        uint32_t descriptorSetLayoutCount;
        std::array<VkPipeline, cMaxSampleOptions> pipelines;
    };

    VkDevice mDevice{VK_NULL_HANDLE};
    foeGfxVkBuiltinDescriptorSets *mBuiltinDescriptorSets{nullptr};

    std::vector<PipelineSet> mPipelines;
};

#endif // FOE_GRAPHICS_VK_PIPELINE_POOL_HPP