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

#include <foe/graphics/vk/pipeline_pool.hpp>

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <vk_error_code.hpp>

#include <array>

#include "log.hpp"
#include "session.hpp"

VkResult foeGfxVkPipelinePool::initialize(
    foeGfxSession session, foeBuiltinDescriptorSets *pBuiltinDescriptorSets) noexcept {
    if (initialized())
        return VK_ERROR_INITIALIZATION_FAILED;

    auto *pSession = session_from_handle(session);

    mDevice = pSession->device;
    mBuiltinDescriptorSets = pBuiltinDescriptorSets;

    return VK_SUCCESS;
}

void foeGfxVkPipelinePool::deinitialize() noexcept {
    for (auto &it : mPipelines) {
        if (it.pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(mDevice, it.pipeline, nullptr);
        if (it.layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(mDevice, it.layout, nullptr);
    }

    mBuiltinDescriptorSets = nullptr;
    mDevice = VK_NULL_HANDLE;
}

bool foeGfxVkPipelinePool::initialized() const noexcept { return mDevice != VK_NULL_HANDLE; }

VkResult foeGfxVkPipelinePool::getPipeline(foeVertexDescriptor *vertexDescriptor,
                                           foeFragmentDescriptor *fragmentDescriptor,
                                           VkRenderPass renderPass,
                                           uint32_t subpass,
                                           VkPipelineLayout *pPipelineLayout,
                                           uint32_t *pDescriptorSetLayoutCount,
                                           VkPipeline *pPipeline) {
    // Try to retrieve an already-created pipeline
    for (auto const &pipeline : mPipelines) {
        if (pipeline.vertexDescriptor == vertexDescriptor &&
            pipeline.fragmentDescriptor == fragmentDescriptor &&
            pipeline.renderPass == renderPass && pipeline.subpass == subpass) {

            *pPipelineLayout = pipeline.layout;
            *pDescriptorSetLayoutCount = pipeline.descriptorSetLayoutCount;
            *pPipeline = pipeline.pipeline;

            return VK_SUCCESS;
        }
    }

    // Generate a new pipeline
    FOE_LOG(foeVkGraphics, Verbose, "Generating a new VkPipeline")

    VkResult res = createPipeline(vertexDescriptor, fragmentDescriptor, renderPass, subpass,
                                  pPipelineLayout, pDescriptorSetLayoutCount, pPipeline);
    if (res == VK_SUCCESS) {
        mPipelines.emplace_back(Pipeline{
            .vertexDescriptor = vertexDescriptor,
            .fragmentDescriptor = fragmentDescriptor,
            .renderPass = renderPass,
            .subpass = subpass,
            .layout = *pPipelineLayout,
            .descriptorSetLayoutCount = *pDescriptorSetLayoutCount,
            .pipeline = *pPipeline,
        });
    }

    return res;
}

VkResult foeGfxVkPipelinePool::createPipeline(foeVertexDescriptor *vertexDescriptor,
                                              foeFragmentDescriptor *fragmentDescriptor,
                                              VkRenderPass renderPass,
                                              uint32_t subpass,
                                              VkPipelineLayout *pPipelineLayout,
                                              uint32_t *pDescriptorSetLayoutCount,
                                              VkPipeline *pPipeline) const noexcept {
    VkResult res{VK_SUCCESS};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    uint32_t descriptorSetLayoutCount{0};
    VkPipeline pipeline{VK_NULL_HANDLE};

    { // Pipeline Layout
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;

        { // Builtin Descriptor Set Layouts
            auto builtinLayouts = vertexDescriptor->getBuiltinSetLayouts() |
                                  fragmentDescriptor->getBuiltinSetLayouts();

            for (int i = 0; i < std::numeric_limits<foeBuiltinDescriptorSetLayoutFlags>::digits;
                 ++i) {
                foeBuiltinDescriptorSetLayoutFlags j = 1U << i;
                if ((j & builtinLayouts) != 0) {
                    // Used builtin layout
                    auto idx = mBuiltinDescriptorSets->getBuiltinSetLayoutIndex(j);
                    if (descriptorSetLayouts.size() <= idx) {
                        descriptorSetLayouts.resize(j + 1);
                    }

                    descriptorSetLayouts[idx] = mBuiltinDescriptorSets->getBuiltinLayout(j);
                }
            }
        }

        // Vertex Descriptor
        if (vertexDescriptor->mVertex != nullptr) {
            if (vertexDescriptor->mVertex->layout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::VertexShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::VertexShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::VertexShader] =
                    vertexDescriptor->mVertex->layout;

                if (vertexDescriptor->mVertex->pushConstantRange.size > 0) {
                    auto temp = vertexDescriptor->mVertex->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mTessellationControl != nullptr) {
            if (vertexDescriptor->mTessellationControl->layout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <=
                    foeDescriptorSetLayoutIndex::TessellationControlShader) {
                    descriptorSetLayouts.resize(
                        foeDescriptorSetLayoutIndex::TessellationControlShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::TessellationControlShader] =
                    vertexDescriptor->mTessellationControl->layout;

                if (vertexDescriptor->mTessellationControl->pushConstantRange.size > 0) {
                    auto temp = vertexDescriptor->mTessellationControl->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mTessellationEvaluation != nullptr) {
            if (vertexDescriptor->mTessellationEvaluation->layout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <=
                    foeDescriptorSetLayoutIndex::TessellationEvaluationShader) {
                    descriptorSetLayouts.resize(
                        foeDescriptorSetLayoutIndex::TessellationEvaluationShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::TessellationEvaluationShader] =
                    vertexDescriptor->mTessellationEvaluation->layout;

                if (vertexDescriptor->mTessellationEvaluation->pushConstantRange.size > 0) {
                    auto temp = vertexDescriptor->mTessellationEvaluation->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mGeometry != nullptr) {
            if (vertexDescriptor->mGeometry->layout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::GeometryShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::GeometryShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::GeometryShader] =
                    vertexDescriptor->mGeometry->layout;

                if (vertexDescriptor->mGeometry->pushConstantRange.size > 0) {
                    auto temp = vertexDescriptor->mGeometry->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        // Fragment Descriptor
        if (fragmentDescriptor->mFragment != nullptr) {
            if (fragmentDescriptor->mFragment->layout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::FragmentShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::FragmentShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::FragmentShader] =
                    fragmentDescriptor->mFragment->layout;

                if (fragmentDescriptor->mFragment->pushConstantRange.size > 0) {
                    auto temp = fragmentDescriptor->mFragment->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        // Fill in dummy layouts
        auto dummyLayout = mBuiltinDescriptorSets->getDummyLayout();
        for (auto &it : descriptorSetLayouts) {
            if (it == VK_NULL_HANDLE)
                it = dummyLayout;
        }

        // Create the layout
        VkPipelineLayoutCreateInfo pipelineLayoutCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
            .pPushConstantRanges = pushConstantRanges.data(),
        };

        descriptorSetLayoutCount = descriptorSetLayouts.size();
        res = vkCreatePipelineLayout(mDevice, &pipelineLayoutCI, nullptr, &pipelineLayout);
        if (res != VK_SUCCESS) {
            FOE_LOG(foeVkGraphics, Error, "Failed to generate VkPipelineLayout with error: {}",
                    std::error_code{res}.message())
            goto CREATE_FAILED;
        }
    }

    { // Pipeline
        // Shader Stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderList;

        // Vertex Descriptor
        if (vertexDescriptor->mVertex != nullptr) {
            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertexDescriptor->mVertex->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mTessellationControl != nullptr) {
            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                .module = vertexDescriptor->mTessellationControl->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mTessellationEvaluation != nullptr) {
            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                .module = vertexDescriptor->mTessellationEvaluation->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mGeometry != nullptr) {
            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
                .module = vertexDescriptor->mGeometry->module,
                .pName = "main",
            });
        }

        // Fragment Descriptor
        if (fragmentDescriptor->mFragment != nullptr) {
            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragmentDescriptor->mFragment->module,
                .pName = "main",
            });
        }

        // Viewport
        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        // Dynamic State
        std::array<VkDynamicState, 2> dynamicStateEnable = {VK_DYNAMIC_STATE_VIEWPORT,
                                                            VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = dynamicStateEnable.size(),
            .pDynamicStates = dynamicStateEnable.data(),
        };

        // Multisample State
        VkPipelineMultisampleStateCreateInfo multisampleState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        // Pipeline
        VkGraphicsPipelineCreateInfo pipelineCI{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaderList.size()),
            .pStages = shaderList.data(),
            .pVertexInputState = vertexDescriptor->getVertexInputSCI(),
            .pInputAssemblyState = &vertexDescriptor->mInputAssemblySCI,
            .pTessellationState = vertexDescriptor->getTessellationSCI(),
            .pViewportState = &viewportState,
            .pRasterizationState = &fragmentDescriptor->mRasterizationSCI,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &fragmentDescriptor->mDepthStencilSCI,
            .pColorBlendState = fragmentDescriptor->getColourBlendSCI(),
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = subpass,
        };

        res =
            vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
        if (res != VK_SUCCESS) {
            FOE_LOG(foeVkGraphics, Error, "Failed to generate VkPipeline with error: {}",
                    std::error_code{res}.message())
        }
    }

CREATE_FAILED:
    if (res != VK_SUCCESS) {
        if (pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(mDevice, pipeline, nullptr);

        if (pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(mDevice, pipelineLayout, nullptr);
    } else {
        *pPipelineLayout = pipelineLayout;
        *pDescriptorSetLayoutCount = descriptorSetLayoutCount;
        *pPipeline = pipeline;
    }

    return res;
}