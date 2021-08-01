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

#ifndef FOE_GRAPHICS_VK_PIPELINE_POOL_HPP
#define FOE_GRAPHICS_VK_PIPELINE_POOL_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/session.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

struct foeGfxVertexDescriptor;
struct foeGfxVkFragmentDescriptor;
class foeGfxVkBuiltinDescriptorSets;

class foeGfxVkPipelinePool {
  public:
    FOE_GFX_EXPORT VkResult initialize(foeGfxSession session) noexcept;
    FOE_GFX_EXPORT void deinitialize() noexcept;
    FOE_GFX_EXPORT bool initialized() const noexcept;

    FOE_GFX_EXPORT VkResult getPipeline(foeGfxVertexDescriptor *vertexDescriptor,
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