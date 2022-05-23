/*
    Copyright (C) 2020-2022 George Cave.

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

#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/graphics/vk/vertex_descriptor.hpp>

#include <array>

#include "builtin_descriptor_sets.hpp"
#include "log.hpp"
#include "result.h"
#include "session.hpp"
#include "shader.hpp"
#include "vk_result.h"

foeResult foeGfxVkPipelinePool::initialize(foeGfxSession session) noexcept {
    if (initialized())
        return vk_to_foeResult(VK_ERROR_INITIALIZATION_FAILED);

    auto *pSession = session_from_handle(session);

    mDevice = pSession->device;
    mBuiltinDescriptorSets = &pSession->builtinDescriptorSets;

    return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
}

void foeGfxVkPipelinePool::deinitialize() noexcept {
    for (auto &it : mPipelines) {
        for (auto pipeline : it.pipelines) {
            if (pipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(mDevice, pipeline, nullptr);
        }
        if (it.layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(mDevice, it.layout, nullptr);
    }

    mBuiltinDescriptorSets = nullptr;
    mDevice = VK_NULL_HANDLE;
}

bool foeGfxVkPipelinePool::initialized() const noexcept { return mDevice != VK_NULL_HANDLE; }

namespace {

size_t sampleCountIndex(VkSampleCountFlags samples) {
    switch (samples) {
    case VK_SAMPLE_COUNT_1_BIT:
        return 0;
    case VK_SAMPLE_COUNT_2_BIT:
        return 1;
    case VK_SAMPLE_COUNT_4_BIT:
        return 2;
    case VK_SAMPLE_COUNT_8_BIT:
        return 3;
    case VK_SAMPLE_COUNT_16_BIT:
        return 4;
    case VK_SAMPLE_COUNT_32_BIT:
        return 5;
    case VK_SAMPLE_COUNT_64_BIT:
        return 6;

    default:
        FOE_LOG(foeVkGraphics, Warning, "Failed to determine given sample count, defaulting to 1")
        return 0;
    }
}

} // namespace

foeResult foeGfxVkPipelinePool::getPipeline(foeGfxVertexDescriptor *vertexDescriptor,
                                            foeGfxVkFragmentDescriptor *fragmentDescriptor,
                                            VkRenderPass renderPass,
                                            uint32_t subpass,
                                            VkSampleCountFlags samples,
                                            VkPipelineLayout *pPipelineLayout,
                                            uint32_t *pDescriptorSetLayoutCount,
                                            VkPipeline *pPipeline) {
    auto sampleIndex = sampleCountIndex(samples);
    PipelineSet *pPipelineSet{nullptr};

    // Try to retrieve an already-created pipeline
    for (auto &pipeline : mPipelines) {
        if (pipeline.vertexDescriptor == vertexDescriptor &&
            pipeline.fragmentDescriptor == fragmentDescriptor &&
            pipeline.renderPass == renderPass && pipeline.subpass == subpass) {
            if (pipeline.pipelines[sampleIndex] != VK_NULL_HANDLE) {
                // If the pipeline exists for the specified sample count, use it
                *pPipelineLayout = pipeline.layout;
                *pDescriptorSetLayoutCount = pipeline.descriptorSetLayoutCount;
                *pPipeline = pipeline.pipelines[sampleIndex];

                return to_foeResult(FOE_GRAPHICS_VK_SUCCESS);
            } else {
                // Otherwise, we'll be creating just a new pipeline for it
                pPipelineSet = &pipeline;
                break;
            }
        }
    }

    // Generate a new pipeline
    FOE_LOG(foeVkGraphics, Verbose, "Generating a new Graphics Pipeline")

    VkResult vkResult =
        createPipeline(vertexDescriptor, fragmentDescriptor, renderPass, subpass, samples,
                       pPipelineLayout, pDescriptorSetLayoutCount, pPipeline);
    if (vkResult == VK_SUCCESS) {
        if (pPipelineSet != nullptr) {
            // Already have the set available, just need the specific sample count variant
            pPipelineSet->pipelines[sampleIndex] = *pPipeline;
        } else {
            // No pipeline like it yet, create new.
            PipelineSet newEntry{
                .vertexDescriptor = vertexDescriptor,
                .fragmentDescriptor = fragmentDescriptor,
                .renderPass = renderPass,
                .subpass = subpass,
                .layout = *pPipelineLayout,
                .descriptorSetLayoutCount = *pDescriptorSetLayoutCount,
            };
            // Make sure the pipeline goes to the specific sample count entry
            newEntry.pipelines[sampleIndex] = *pPipeline;

            mPipelines.emplace_back(newEntry);
        }
    }

    return vk_to_foeResult(vkResult);
}

VkResult foeGfxVkPipelinePool::createPipeline(foeGfxVertexDescriptor *vertexDescriptor,
                                              foeGfxVkFragmentDescriptor *fragmentDescriptor,
                                              VkRenderPass renderPass,
                                              uint32_t subpass,
                                              VkSampleCountFlags samples,
                                              VkPipelineLayout *pPipelineLayout,
                                              uint32_t *pDescriptorSetLayoutCount,
                                              VkPipeline *pPipeline) const noexcept {
    VkResult vkResult{VK_SUCCESS};
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
            auto *pShader = shader_from_handle(vertexDescriptor->mVertex);

            if (pShader->descriptorSetLayout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::VertexShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::VertexShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::VertexShader] =
                    pShader->descriptorSetLayout;

                if (pShader->pushConstantRange.size > 0) {
                    auto temp = pShader->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mTessellationControl != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mTessellationControl);

            if (pShader->descriptorSetLayout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <=
                    foeDescriptorSetLayoutIndex::TessellationControlShader) {
                    descriptorSetLayouts.resize(
                        foeDescriptorSetLayoutIndex::TessellationControlShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::TessellationControlShader] =
                    pShader->descriptorSetLayout;

                if (pShader->pushConstantRange.size > 0) {
                    auto temp = pShader->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mTessellationEvaluation != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mTessellationEvaluation);

            if (pShader->descriptorSetLayout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <=
                    foeDescriptorSetLayoutIndex::TessellationEvaluationShader) {
                    descriptorSetLayouts.resize(
                        foeDescriptorSetLayoutIndex::TessellationEvaluationShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::TessellationEvaluationShader] =
                    pShader->descriptorSetLayout;

                if (pShader->pushConstantRange.size > 0) {
                    auto temp = pShader->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        if (vertexDescriptor->mGeometry != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mGeometry);

            if (pShader->descriptorSetLayout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::GeometryShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::GeometryShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::GeometryShader] =
                    pShader->descriptorSetLayout;

                if (pShader->pushConstantRange.size > 0) {
                    auto temp = pShader->pushConstantRange;
                    temp.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
                    pushConstantRanges.emplace_back(temp);
                }
            }
        }

        // Fragment Descriptor
        if (fragmentDescriptor->mFragment != FOE_NULL_HANDLE) {
            auto *pShader = shader_from_handle(fragmentDescriptor->mFragment);

            if (pShader->descriptorSetLayout != VK_NULL_HANDLE) {
                if (descriptorSetLayouts.size() <= foeDescriptorSetLayoutIndex::FragmentShader) {
                    descriptorSetLayouts.resize(foeDescriptorSetLayoutIndex::FragmentShader + 1);
                }
                descriptorSetLayouts[foeDescriptorSetLayoutIndex::FragmentShader] =
                    pShader->descriptorSetLayout;

                if (pShader->pushConstantRange.size > 0) {
                    auto temp = pShader->pushConstantRange;
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

        descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        vkResult = vkCreatePipelineLayout(mDevice, &pipelineLayoutCI, nullptr, &pipelineLayout);
        if (vkResult != VK_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            VkResultToString(vkResult, buffer);
            FOE_LOG(foeVkGraphics, Error, "Failed to generate VkPipelineLayout with error: {}",
                    buffer)

            goto CREATE_FAILED;
        }
    }

    { // Pipeline
        // Shader Stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderList;

        // Vertex Descriptor
        if (vertexDescriptor->mVertex != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mVertex);

            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = pShader->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mTessellationControl != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mTessellationControl);

            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                .module = pShader->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mTessellationEvaluation != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mTessellationEvaluation);

            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                .module = pShader->module,
                .pName = "main",
            });
        }

        if (vertexDescriptor->mGeometry != nullptr) {
            auto *pShader = shader_from_handle(vertexDescriptor->mGeometry);

            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
                .module = pShader->module,
                .pName = "main",
            });
        }

        // Fragment Descriptor
        if (fragmentDescriptor->mFragment != FOE_NULL_HANDLE) {
            auto *pShader = shader_from_handle(fragmentDescriptor->mFragment);

            shaderList.emplace_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = pShader->module,
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
            .dynamicStateCount = static_cast<uint32_t>(dynamicStateEnable.size()),
            .pDynamicStates = dynamicStateEnable.data(),
        };

        // Multisample State
        VkPipelineMultisampleStateCreateInfo multisampleState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = static_cast<VkSampleCountFlagBits>(samples),
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
            .pRasterizationState = (fragmentDescriptor->hasRasterizationSCI)
                                       ? &fragmentDescriptor->mRasterizationSCI
                                       : nullptr,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = (fragmentDescriptor->hasDepthStencilSCI)
                                      ? &fragmentDescriptor->mDepthStencilSCI
                                      : nullptr,
            .pColorBlendState = fragmentDescriptor->getColourBlendSCI(),
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = subpass,
        };

        vkResult =
            vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
        if (vkResult != VK_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            VkResultToString(vkResult, buffer);
            FOE_LOG(foeVkGraphics, Error, "Failed to generate VkPipeline with error: {}", buffer)
        }
    }

CREATE_FAILED:
    if (vkResult != VK_SUCCESS) {
        if (pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(mDevice, pipeline, nullptr);

        if (pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(mDevice, pipelineLayout, nullptr);
    } else {
        *pPipelineLayout = pipelineLayout;
        *pDescriptorSetLayoutCount = descriptorSetLayoutCount;
        *pPipeline = pipeline;
    }

    return vkResult;
}