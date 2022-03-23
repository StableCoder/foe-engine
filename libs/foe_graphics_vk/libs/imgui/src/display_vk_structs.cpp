/*
    Copyright (C) 2022 George Cave.

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

// NOTE: This file was auto-generated by generate_vk_imgui_code.py

#include <foe/graphics/vk/imgui/display_vk_structs.hpp>

#include <foe/graphics/vk/imgui/display_vk_enums.hpp>
#include <foe/graphics/vk/imgui/display_vk_types.hpp>
#include <foe/imgui/plain_old_data.hpp>
#include <imgui.h>

void imgui_VkDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding const &data) {

    // uint32_t - binding
    imgui_pod<uint32_t>("binding", data.binding);

    // VkDescriptorType - descriptorType
    imgui_VkDescriptorType("descriptorType (VkDescriptorType)", data.descriptorType);

    // uint32_t - descriptorCount
    imgui_pod<uint32_t>("descriptorCount", data.descriptorCount);

    // VkShaderStageFlags - stageFlags
    imgui_VkShaderStageFlags("stageFlags (VkShaderStageFlags)", data.stageFlags);
}

void imgui_VkDescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo const &data) {

    // VkDescriptorSetLayoutCreateFlags - flags
    imgui_VkDescriptorSetLayoutCreateFlags("flags (VkDescriptorSetLayoutCreateFlags)", data.flags);

    // VkDescriptorSetLayoutBinding - pBindings / bindingCount
    if (std::string nodeName =
            "pBindings:" + std::to_string(data.bindingCount) + " (VkDescriptorSetLayoutBinding)";
        ImGui::TreeNode(nodeName.c_str())) {
        for (size_t i = 0; i < data.bindingCount; ++i) {
            auto const &it = data.pBindings[i];
            imgui_VkDescriptorSetLayoutBinding(it);
        }

        ImGui::TreePop();
    }
}

void imgui_VkVertexInputBindingDescription(VkVertexInputBindingDescription const &data) {

    // uint32_t - binding
    imgui_pod<uint32_t>("binding", data.binding);

    // uint32_t - stride
    imgui_pod<uint32_t>("stride", data.stride);

    // VkVertexInputRate - inputRate
    imgui_VkVertexInputRate("inputRate (VkVertexInputRate)", data.inputRate);
}

void imgui_VkVertexInputAttributeDescription(VkVertexInputAttributeDescription const &data) {

    // uint32_t - location
    imgui_pod<uint32_t>("location", data.location);

    // uint32_t - binding
    imgui_pod<uint32_t>("binding", data.binding);

    // VkFormat - format
    imgui_VkFormat("format (VkFormat)", data.format);

    // uint32_t - offset
    imgui_pod<uint32_t>("offset", data.offset);
}

void imgui_VkPipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo const &data) {

    // VkPipelineVertexInputStateCreateFlags - flags
    imgui_VkPipelineVertexInputStateCreateFlags("flags (VkPipelineVertexInputStateCreateFlags)",
                                                data.flags);
}

void imgui_VkPipelineInputAssemblyStateCreateInfo(
    VkPipelineInputAssemblyStateCreateInfo const &data) {

    // VkPipelineInputAssemblyStateCreateFlags - flags
    imgui_VkPipelineInputAssemblyStateCreateFlags("flags (VkPipelineInputAssemblyStateCreateFlags)",
                                                  data.flags);

    // VkPrimitiveTopology - topology
    imgui_VkPrimitiveTopology("topology (VkPrimitiveTopology)", data.topology);

    // VkBool32 - primitiveRestartEnable
    imgui_VkBool32("primitiveRestartEnable (VkBool32)", data.primitiveRestartEnable);
}

void imgui_VkPipelineTessellationStateCreateInfo(
    VkPipelineTessellationStateCreateInfo const &data) {

    // VkPipelineTessellationStateCreateFlags - flags
    imgui_VkPipelineTessellationStateCreateFlags("flags (VkPipelineTessellationStateCreateFlags)",
                                                 data.flags);

    // uint32_t - patchControlPoints
    imgui_pod<uint32_t>("patchControlPoints", data.patchControlPoints);
}

void imgui_VkPipelineRasterizationStateCreateInfo(
    VkPipelineRasterizationStateCreateInfo const &data) {

    // VkPipelineRasterizationStateCreateFlags - flags
    imgui_VkPipelineRasterizationStateCreateFlags("flags (VkPipelineRasterizationStateCreateFlags)",
                                                  data.flags);

    // VkBool32 - depthClampEnable
    imgui_VkBool32("depthClampEnable (VkBool32)", data.depthClampEnable);

    // VkBool32 - rasterizerDiscardEnable
    imgui_VkBool32("rasterizerDiscardEnable (VkBool32)", data.rasterizerDiscardEnable);

    // VkPolygonMode - polygonMode
    imgui_VkPolygonMode("polygonMode (VkPolygonMode)", data.polygonMode);

    // VkCullModeFlags - cullMode
    imgui_VkCullModeFlags("cullMode (VkCullModeFlags)", data.cullMode);

    // VkFrontFace - frontFace
    imgui_VkFrontFace("frontFace (VkFrontFace)", data.frontFace);

    // VkBool32 - depthBiasEnable
    imgui_VkBool32("depthBiasEnable (VkBool32)", data.depthBiasEnable);

    // float - depthBiasConstantFactor
    imgui_pod<float>("depthBiasConstantFactor", data.depthBiasConstantFactor);

    // float - depthBiasClamp
    imgui_pod<float>("depthBiasClamp", data.depthBiasClamp);

    // float - depthBiasSlopeFactor
    imgui_pod<float>("depthBiasSlopeFactor", data.depthBiasSlopeFactor);

    // float - lineWidth
    imgui_pod<float>("lineWidth", data.lineWidth);
}

void imgui_VkPipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState const &data) {

    // VkBool32 - blendEnable
    imgui_VkBool32("blendEnable (VkBool32)", data.blendEnable);

    // VkBlendFactor - srcColorBlendFactor
    imgui_VkBlendFactor("srcColorBlendFactor (VkBlendFactor)", data.srcColorBlendFactor);

    // VkBlendFactor - dstColorBlendFactor
    imgui_VkBlendFactor("dstColorBlendFactor (VkBlendFactor)", data.dstColorBlendFactor);

    // VkBlendOp - colorBlendOp
    imgui_VkBlendOp("colorBlendOp (VkBlendOp)", data.colorBlendOp);

    // VkBlendFactor - srcAlphaBlendFactor
    imgui_VkBlendFactor("srcAlphaBlendFactor (VkBlendFactor)", data.srcAlphaBlendFactor);

    // VkBlendFactor - dstAlphaBlendFactor
    imgui_VkBlendFactor("dstAlphaBlendFactor (VkBlendFactor)", data.dstAlphaBlendFactor);

    // VkBlendOp - alphaBlendOp
    imgui_VkBlendOp("alphaBlendOp (VkBlendOp)", data.alphaBlendOp);

    // VkColorComponentFlags - colorWriteMask
    imgui_VkColorComponentFlags("colorWriteMask (VkColorComponentFlags)", data.colorWriteMask);
}

void imgui_VkPipelineColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo const &data) {

    // VkPipelineColorBlendStateCreateFlags - flags
    imgui_VkPipelineColorBlendStateCreateFlags("flags (VkPipelineColorBlendStateCreateFlags)",
                                               data.flags);

    // VkBool32 - logicOpEnable
    imgui_VkBool32("logicOpEnable (VkBool32)", data.logicOpEnable);

    // VkLogicOp - logicOp
    imgui_VkLogicOp("logicOp (VkLogicOp)", data.logicOp);

    // VkPipelineColorBlendAttachmentState - pAttachments / attachmentCount
    if (std::string nodeName = "pAttachments:" + std::to_string(data.attachmentCount) +
                               " (VkPipelineColorBlendAttachmentState)";
        ImGui::TreeNode(nodeName.c_str())) {
        for (size_t i = 0; i < data.attachmentCount; ++i) {
            auto const &it = data.pAttachments[i];
            imgui_VkPipelineColorBlendAttachmentState(it);
        }

        ImGui::TreePop();
    }
}

void imgui_VkStencilOpState(VkStencilOpState const &data) {

    // VkStencilOp - failOp
    imgui_VkStencilOp("failOp (VkStencilOp)", data.failOp);

    // VkStencilOp - passOp
    imgui_VkStencilOp("passOp (VkStencilOp)", data.passOp);

    // VkStencilOp - depthFailOp
    imgui_VkStencilOp("depthFailOp (VkStencilOp)", data.depthFailOp);

    // VkCompareOp - compareOp
    imgui_VkCompareOp("compareOp (VkCompareOp)", data.compareOp);

    // uint32_t - compareMask
    imgui_pod<uint32_t>("compareMask", data.compareMask);

    // uint32_t - writeMask
    imgui_pod<uint32_t>("writeMask", data.writeMask);

    // uint32_t - reference
    imgui_pod<uint32_t>("reference", data.reference);
}

void imgui_VkPipelineDepthStencilStateCreateInfo(
    VkPipelineDepthStencilStateCreateInfo const &data) {

    // VkPipelineDepthStencilStateCreateFlags - flags
    imgui_VkPipelineDepthStencilStateCreateFlags("flags (VkPipelineDepthStencilStateCreateFlags)",
                                                 data.flags);

    // VkBool32 - depthTestEnable
    imgui_VkBool32("depthTestEnable (VkBool32)", data.depthTestEnable);

    // VkBool32 - depthWriteEnable
    imgui_VkBool32("depthWriteEnable (VkBool32)", data.depthWriteEnable);

    // VkCompareOp - depthCompareOp
    imgui_VkCompareOp("depthCompareOp (VkCompareOp)", data.depthCompareOp);

    // VkBool32 - depthBoundsTestEnable
    imgui_VkBool32("depthBoundsTestEnable (VkBool32)", data.depthBoundsTestEnable);

    // VkBool32 - stencilTestEnable
    imgui_VkBool32("stencilTestEnable (VkBool32)", data.stencilTestEnable);

    // VkStencilOpState - front
    if (ImGui::TreeNode("front (VkStencilOpState)")) {
        imgui_VkStencilOpState(data.front);
        ImGui::TreePop();
    }

    // VkStencilOpState - back
    if (ImGui::TreeNode("back (VkStencilOpState)")) {
        imgui_VkStencilOpState(data.back);
        ImGui::TreePop();
    }

    // float - minDepthBounds
    imgui_pod<float>("minDepthBounds", data.minDepthBounds);

    // float - maxDepthBounds
    imgui_pod<float>("maxDepthBounds", data.maxDepthBounds);
}

void imgui_VkPushConstantRange(VkPushConstantRange const &data) {

    // VkShaderStageFlags - stageFlags
    imgui_VkShaderStageFlags("stageFlags (VkShaderStageFlags)", data.stageFlags);

    // uint32_t - offset
    imgui_pod<uint32_t>("offset", data.offset);

    // uint32_t - size
    imgui_pod<uint32_t>("size", data.size);
}
