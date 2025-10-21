// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/external/vk_struct_cleanup.h>
#include <foe/external/vk_struct_compare.h>
#include <foe/graphics/vk/yaml/vk_enums.hpp>
#include <foe/graphics/vk/yaml/vk_pod.hpp>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include <string.h>

bool yaml_read_VkPushConstantRange(std::string const &nodeName,
                                   YAML::Node const &node,
                                   VkPushConstantRange &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPushConstantRange newData;
    memset(&newData, 0, sizeof(VkPushConstantRange));
    try {
        // VkShaderStageFlags - stageFlags
        yaml_read_VkEnum("VkShaderStageFlags", "stage_flags", readNode, newData.stageFlags);

        // uint32_t - offset
        yaml_read_uint32_t("offset", readNode, newData.offset);

        // uint32_t - size
        yaml_read_uint32_t("size", readNode, newData.size);
    } catch (foeYamlException const &e) {
        cleanup_VkPushConstantRange(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPushConstantRange(std::string const &nodeName,
                                    VkPushConstantRange const &data,
                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkShaderStageFlags - stageFlags
        if (data.stageFlags != 0) {
            yaml_write_VkEnum("VkShaderStageFlags", "stage_flags", data.stageFlags, writeNode);
        }

        // uint32_t - offset
        if (data.offset != 0) {
            yaml_write_uint32_t("offset", data.offset, writeNode);
        }

        // uint32_t - size
        if (data.size != 0) {
            yaml_write_uint32_t("size", data.size, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkDescriptorSetLayoutBinding(std::string const &nodeName,
                                            YAML::Node const &node,
                                            VkDescriptorSetLayoutBinding &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkDescriptorSetLayoutBinding newData;
    memset(&newData, 0, sizeof(VkDescriptorSetLayoutBinding));
    try {
        // uint32_t - binding
        yaml_read_uint32_t("binding", readNode, newData.binding);

        // VkDescriptorType - descriptorType
        yaml_read_VkEnum("VkDescriptorType", "descriptor_type", readNode, newData.descriptorType);

        // uint32_t - descriptorCount
        yaml_read_uint32_t("descriptor_count", readNode, newData.descriptorCount);

        // VkShaderStageFlags - stageFlags
        yaml_read_VkEnum("VkShaderStageFlags", "stage_flags", readNode, newData.stageFlags);
    } catch (foeYamlException const &e) {
        cleanup_VkDescriptorSetLayoutBinding(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkDescriptorSetLayoutBinding(std::string const &nodeName,
                                             VkDescriptorSetLayoutBinding const &data,
                                             YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // uint32_t - binding
        if (data.binding != 0) {
            yaml_write_uint32_t("binding", data.binding, writeNode);
        }

        // VkDescriptorType - descriptorType
        yaml_write_VkEnum("VkDescriptorType", "descriptor_type", data.descriptorType, writeNode);

        // uint32_t - descriptorCount
        if (data.descriptorCount != 0) {
            yaml_write_uint32_t("descriptor_count", data.descriptorCount, writeNode);
        }

        // VkShaderStageFlags - stageFlags
        if (data.stageFlags != 0) {
            yaml_write_VkEnum("VkShaderStageFlags", "stage_flags", data.stageFlags, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkDescriptorSetLayoutCreateInfo(std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkDescriptorSetLayoutCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkDescriptorSetLayoutCreateInfo newData;
    memset(&newData, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        // VkDescriptorSetLayoutCreateFlags - flags
        yaml_read_VkEnum("VkDescriptorSetLayoutCreateFlags", "flags", readNode, newData.flags);

        // VkDescriptorSetLayoutBinding* - pBindings[bindingCount]
        if (YAML::Node pBindingsNode = readNode["bindings"]; pBindingsNode) {
            newData.bindingCount = pBindingsNode.size();
            if (newData.bindingCount > 0) {
                newData.pBindings = (VkDescriptorSetLayoutBinding *)malloc(
                    newData.bindingCount * sizeof(VkDescriptorSetLayoutBinding));
                for (size_t i = 0; i < newData.bindingCount; ++i) {
                    YAML::Node subReadNode = pBindingsNode[i];
                    if (!yaml_read_VkDescriptorSetLayoutBinding(
                            "", subReadNode,
                            (VkDescriptorSetLayoutBinding &)newData.pBindings[i])) {
                        throw foeYamlException{"bindings - Failed to read list-node"};
                    }
                }
            }
        }
    } catch (foeYamlException const &e) {
        cleanup_VkDescriptorSetLayoutCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkDescriptorSetLayoutCreateInfo(std::string const &nodeName,
                                                VkDescriptorSetLayoutCreateInfo const &data,
                                                YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkDescriptorSetLayoutCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkDescriptorSetLayoutCreateFlags", "flags", data.flags, writeNode);
        }

        // VkDescriptorSetLayoutBinding* - pBindings[bindingCount]
        if (data.bindingCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.bindingCount; ++i) {
                YAML::Node newNode;
                yaml_write_VkDescriptorSetLayoutBinding("", data.pBindings[i], newNode);
                subWriteNode.push_back(newNode);
            }

            writeNode["bindings"] = subWriteNode;
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkStencilOpState(std::string const &nodeName,
                                YAML::Node const &node,
                                VkStencilOpState &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkStencilOpState newData;
    memset(&newData, 0, sizeof(VkStencilOpState));
    try {
        // VkStencilOp - failOp
        yaml_read_VkEnum("VkStencilOp", "fail_op", readNode, newData.failOp);

        // VkStencilOp - passOp
        yaml_read_VkEnum("VkStencilOp", "pass_op", readNode, newData.passOp);

        // VkStencilOp - depthFailOp
        yaml_read_VkEnum("VkStencilOp", "depth_fail_op", readNode, newData.depthFailOp);

        // VkCompareOp - compareOp
        yaml_read_VkEnum("VkCompareOp", "compare_op", readNode, newData.compareOp);

        // uint32_t - compareMask
        yaml_read_uint32_t("compare_mask", readNode, newData.compareMask);

        // uint32_t - writeMask
        yaml_read_uint32_t("write_mask", readNode, newData.writeMask);

        // uint32_t - reference
        yaml_read_uint32_t("reference", readNode, newData.reference);
    } catch (foeYamlException const &e) {
        cleanup_VkStencilOpState(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkStencilOpState(std::string const &nodeName,
                                 VkStencilOpState const &data,
                                 YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStencilOp - failOp
        yaml_write_VkEnum("VkStencilOp", "fail_op", data.failOp, writeNode);

        // VkStencilOp - passOp
        yaml_write_VkEnum("VkStencilOp", "pass_op", data.passOp, writeNode);

        // VkStencilOp - depthFailOp
        yaml_write_VkEnum("VkStencilOp", "depth_fail_op", data.depthFailOp, writeNode);

        // VkCompareOp - compareOp
        yaml_write_VkEnum("VkCompareOp", "compare_op", data.compareOp, writeNode);

        // uint32_t - compareMask
        if (data.compareMask != 0) {
            yaml_write_uint32_t("compare_mask", data.compareMask, writeNode);
        }

        // uint32_t - writeMask
        if (data.writeMask != 0) {
            yaml_write_uint32_t("write_mask", data.writeMask, writeNode);
        }

        // uint32_t - reference
        if (data.reference != 0) {
            yaml_write_uint32_t("reference", data.reference, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineRasterizationStateCreateInfo(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineRasterizationStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineRasterizationStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        // VkPipelineRasterizationStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineRasterizationStateCreateFlags", "flags", readNode,
                         newData.flags);

        // VkBool32 - depthClampEnable
        yaml_read_VkBool32("depth_clamp_enable", readNode, newData.depthClampEnable);

        // VkBool32 - rasterizerDiscardEnable
        yaml_read_VkBool32("rasterizer_discard_enable", readNode, newData.rasterizerDiscardEnable);

        // VkPolygonMode - polygonMode
        yaml_read_VkEnum("VkPolygonMode", "polygon_mode", readNode, newData.polygonMode);

        // VkCullModeFlags - cullMode
        yaml_read_VkEnum("VkCullModeFlags", "cull_mode", readNode, newData.cullMode);

        // VkFrontFace - frontFace
        yaml_read_VkEnum("VkFrontFace", "front_face", readNode, newData.frontFace);

        // VkBool32 - depthBiasEnable
        yaml_read_VkBool32("depth_bias_enable", readNode, newData.depthBiasEnable);

        // float - depthBiasConstantFactor
        yaml_read_float("depth_bias_constant_factor", readNode, newData.depthBiasConstantFactor);

        // float - depthBiasClamp
        yaml_read_float("depth_bias_clamp", readNode, newData.depthBiasClamp);

        // float - depthBiasSlopeFactor
        yaml_read_float("depth_bias_slope_factor", readNode, newData.depthBiasSlopeFactor);

        // float - lineWidth
        yaml_read_float("line_width", readNode, newData.lineWidth);
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineRasterizationStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineRasterizationStateCreateInfo(
    std::string const &nodeName,
    VkPipelineRasterizationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineRasterizationStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineRasterizationStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // VkBool32 - depthClampEnable
        if (data.depthClampEnable != 0) {
            yaml_write_VkBool32("depth_clamp_enable", data.depthClampEnable, writeNode);
        }

        // VkBool32 - rasterizerDiscardEnable
        if (data.rasterizerDiscardEnable != 0) {
            yaml_write_VkBool32("rasterizer_discard_enable", data.rasterizerDiscardEnable,
                                writeNode);
        }

        // VkPolygonMode - polygonMode
        yaml_write_VkEnum("VkPolygonMode", "polygon_mode", data.polygonMode, writeNode);

        // VkCullModeFlags - cullMode
        if (data.cullMode != 0) {
            yaml_write_VkEnum("VkCullModeFlags", "cull_mode", data.cullMode, writeNode);
        }

        // VkFrontFace - frontFace
        yaml_write_VkEnum("VkFrontFace", "front_face", data.frontFace, writeNode);

        // VkBool32 - depthBiasEnable
        if (data.depthBiasEnable != 0) {
            yaml_write_VkBool32("depth_bias_enable", data.depthBiasEnable, writeNode);
        }

        // float - depthBiasConstantFactor
        if (data.depthBiasConstantFactor != 0) {
            yaml_write_float("depth_bias_constant_factor", data.depthBiasConstantFactor, writeNode);
        }

        // float - depthBiasClamp
        if (data.depthBiasClamp != 0) {
            yaml_write_float("depth_bias_clamp", data.depthBiasClamp, writeNode);
        }

        // float - depthBiasSlopeFactor
        if (data.depthBiasSlopeFactor != 0) {
            yaml_write_float("depth_bias_slope_factor", data.depthBiasSlopeFactor, writeNode);
        }

        // float - lineWidth
        if (data.lineWidth != 0) {
            yaml_write_float("line_width", data.lineWidth, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineDepthStencilStateCreateInfo(std::string const &nodeName,
                                                     YAML::Node const &node,
                                                     VkPipelineDepthStencilStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineDepthStencilStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        // VkPipelineDepthStencilStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineDepthStencilStateCreateFlags", "flags", readNode,
                         newData.flags);

        // VkBool32 - depthTestEnable
        yaml_read_VkBool32("depth_test_enable", readNode, newData.depthTestEnable);

        // VkBool32 - depthWriteEnable
        yaml_read_VkBool32("depth_write_enable", readNode, newData.depthWriteEnable);

        // VkCompareOp - depthCompareOp
        yaml_read_VkEnum("VkCompareOp", "depth_compare_op", readNode, newData.depthCompareOp);

        // VkBool32 - depthBoundsTestEnable
        yaml_read_VkBool32("depth_bounds_test_enable", readNode, newData.depthBoundsTestEnable);

        // VkBool32 - stencilTestEnable
        yaml_read_VkBool32("stencil_test_enable", readNode, newData.stencilTestEnable);

        // VkStencilOpState - front
        yaml_read_VkStencilOpState("front", readNode, newData.front);

        // VkStencilOpState - back
        yaml_read_VkStencilOpState("back", readNode, newData.back);

        // float - minDepthBounds
        yaml_read_float("min_depth_bounds", readNode, newData.minDepthBounds);

        // float - maxDepthBounds
        yaml_read_float("max_depth_bounds", readNode, newData.maxDepthBounds);
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineDepthStencilStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineDepthStencilStateCreateInfo(
    std::string const &nodeName,
    VkPipelineDepthStencilStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineDepthStencilStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineDepthStencilStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // VkBool32 - depthTestEnable
        if (data.depthTestEnable != 0) {
            yaml_write_VkBool32("depth_test_enable", data.depthTestEnable, writeNode);
        }

        // VkBool32 - depthWriteEnable
        if (data.depthWriteEnable != 0) {
            yaml_write_VkBool32("depth_write_enable", data.depthWriteEnable, writeNode);
        }

        // VkCompareOp - depthCompareOp
        yaml_write_VkEnum("VkCompareOp", "depth_compare_op", data.depthCompareOp, writeNode);

        // VkBool32 - depthBoundsTestEnable
        if (data.depthBoundsTestEnable != 0) {
            yaml_write_VkBool32("depth_bounds_test_enable", data.depthBoundsTestEnable, writeNode);
        }

        // VkBool32 - stencilTestEnable
        if (data.stencilTestEnable != 0) {
            yaml_write_VkBool32("stencil_test_enable", data.stencilTestEnable, writeNode);
        }

        // VkStencilOpState - front
        if (VkStencilOpState temp = {}; !compare_VkStencilOpState(&data.front, &temp)) {
            yaml_write_VkStencilOpState("front", data.front, writeNode);
        }

        // VkStencilOpState - back
        if (VkStencilOpState temp = {}; !compare_VkStencilOpState(&data.back, &temp)) {
            yaml_write_VkStencilOpState("back", data.back, writeNode);
        }

        // float - minDepthBounds
        if (data.minDepthBounds != 0) {
            yaml_write_float("min_depth_bounds", data.minDepthBounds, writeNode);
        }

        // float - maxDepthBounds
        if (data.maxDepthBounds != 0) {
            yaml_write_float("max_depth_bounds", data.maxDepthBounds, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineColorBlendAttachmentState(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   VkPipelineColorBlendAttachmentState &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineColorBlendAttachmentState newData;
    memset(&newData, 0, sizeof(VkPipelineColorBlendAttachmentState));
    try {
        // VkBool32 - blendEnable
        yaml_read_VkBool32("blend_enable", readNode, newData.blendEnable);

        // VkBlendFactor - srcColorBlendFactor
        yaml_read_VkEnum("VkBlendFactor", "src_color_blend_factor", readNode,
                         newData.srcColorBlendFactor);

        // VkBlendFactor - dstColorBlendFactor
        yaml_read_VkEnum("VkBlendFactor", "dst_color_blend_factor", readNode,
                         newData.dstColorBlendFactor);

        // VkBlendOp - colorBlendOp
        yaml_read_VkEnum("VkBlendOp", "color_blend_op", readNode, newData.colorBlendOp);

        // VkBlendFactor - srcAlphaBlendFactor
        yaml_read_VkEnum("VkBlendFactor", "src_alpha_blend_factor", readNode,
                         newData.srcAlphaBlendFactor);

        // VkBlendFactor - dstAlphaBlendFactor
        yaml_read_VkEnum("VkBlendFactor", "dst_alpha_blend_factor", readNode,
                         newData.dstAlphaBlendFactor);

        // VkBlendOp - alphaBlendOp
        yaml_read_VkEnum("VkBlendOp", "alpha_blend_op", readNode, newData.alphaBlendOp);

        // VkColorComponentFlags - colorWriteMask
        yaml_read_VkEnum("VkColorComponentFlags", "color_write_mask", readNode,
                         newData.colorWriteMask);
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineColorBlendAttachmentState(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineColorBlendAttachmentState(std::string const &nodeName,
                                                    VkPipelineColorBlendAttachmentState const &data,
                                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkBool32 - blendEnable
        if (data.blendEnable != 0) {
            yaml_write_VkBool32("blend_enable", data.blendEnable, writeNode);
        }

        // VkBlendFactor - srcColorBlendFactor
        yaml_write_VkEnum("VkBlendFactor", "src_color_blend_factor", data.srcColorBlendFactor,
                          writeNode);

        // VkBlendFactor - dstColorBlendFactor
        yaml_write_VkEnum("VkBlendFactor", "dst_color_blend_factor", data.dstColorBlendFactor,
                          writeNode);

        // VkBlendOp - colorBlendOp
        yaml_write_VkEnum("VkBlendOp", "color_blend_op", data.colorBlendOp, writeNode);

        // VkBlendFactor - srcAlphaBlendFactor
        yaml_write_VkEnum("VkBlendFactor", "src_alpha_blend_factor", data.srcAlphaBlendFactor,
                          writeNode);

        // VkBlendFactor - dstAlphaBlendFactor
        yaml_write_VkEnum("VkBlendFactor", "dst_alpha_blend_factor", data.dstAlphaBlendFactor,
                          writeNode);

        // VkBlendOp - alphaBlendOp
        yaml_write_VkEnum("VkBlendOp", "alpha_blend_op", data.alphaBlendOp, writeNode);

        // VkColorComponentFlags - colorWriteMask
        if (data.colorWriteMask != 0) {
            yaml_write_VkEnum("VkColorComponentFlags", "color_write_mask", data.colorWriteMask,
                              writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineColorBlendStateCreateInfo(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   VkPipelineColorBlendStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineColorBlendStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        // VkPipelineColorBlendStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineColorBlendStateCreateFlags", "flags", readNode, newData.flags);

        // VkBool32 - logicOpEnable
        yaml_read_VkBool32("logic_op_enable", readNode, newData.logicOpEnable);

        // VkLogicOp - logicOp
        yaml_read_VkEnum("VkLogicOp", "logic_op", readNode, newData.logicOp);

        // VkPipelineColorBlendAttachmentState* - pAttachments[attachmentCount]
        if (YAML::Node pAttachmentsNode = readNode["attachments"]; pAttachmentsNode) {
            newData.attachmentCount = pAttachmentsNode.size();
            if (newData.attachmentCount > 0) {
                newData.pAttachments = (VkPipelineColorBlendAttachmentState *)malloc(
                    newData.attachmentCount * sizeof(VkPipelineColorBlendAttachmentState));
                for (size_t i = 0; i < newData.attachmentCount; ++i) {
                    YAML::Node subReadNode = pAttachmentsNode[i];
                    if (!yaml_read_VkPipelineColorBlendAttachmentState(
                            "", subReadNode,
                            (VkPipelineColorBlendAttachmentState &)newData.pAttachments[i])) {
                        throw foeYamlException{"attachments - Failed to read list-node"};
                    }
                }
            }
        }

        // float[4] - blendConstants
        if (YAML::Node blendConstantsNode = readNode["blend_constants"]; blendConstantsNode) {
            for (size_t i = 0; i < 4; ++i) {
                YAML::Node subReadNode = blendConstantsNode[i];
                if (!yaml_read_float("", subReadNode, (float &)newData.blendConstants[i])) {
                    throw foeYamlException{"blend_constants - Failed to read list-node"};
                }
            }
        }
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineColorBlendStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineColorBlendStateCreateInfo(std::string const &nodeName,
                                                    VkPipelineColorBlendStateCreateInfo const &data,
                                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineColorBlendStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineColorBlendStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // VkBool32 - logicOpEnable
        if (data.logicOpEnable != 0) {
            yaml_write_VkBool32("logic_op_enable", data.logicOpEnable, writeNode);
        }

        // VkLogicOp - logicOp
        yaml_write_VkEnum("VkLogicOp", "logic_op", data.logicOp, writeNode);

        // VkPipelineColorBlendAttachmentState* - pAttachments[attachmentCount]
        if (data.attachmentCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.attachmentCount; ++i) {
                YAML::Node newNode;
                yaml_write_VkPipelineColorBlendAttachmentState("", data.pAttachments[i], newNode);
                subWriteNode.push_back(newNode);
            }

            writeNode["attachments"] = subWriteNode;
        }

        // float[4] - blendConstants
        {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < 4; ++i) {
                YAML::Node newNode;
                yaml_write_float("", data.blendConstants[i], newNode);
                subWriteNode.push_back(newNode);
            }

            writeNode["blend_constants"] = subWriteNode;
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkVertexInputBindingDescription(std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkVertexInputBindingDescription &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkVertexInputBindingDescription newData;
    memset(&newData, 0, sizeof(VkVertexInputBindingDescription));
    try {
        // uint32_t - binding
        yaml_read_uint32_t("binding", readNode, newData.binding);

        // uint32_t - stride
        yaml_read_uint32_t("stride", readNode, newData.stride);

        // VkVertexInputRate - inputRate
        yaml_read_VkEnum("VkVertexInputRate", "input_rate", readNode, newData.inputRate);
    } catch (foeYamlException const &e) {
        cleanup_VkVertexInputBindingDescription(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkVertexInputBindingDescription(std::string const &nodeName,
                                                VkVertexInputBindingDescription const &data,
                                                YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // uint32_t - binding
        if (data.binding != 0) {
            yaml_write_uint32_t("binding", data.binding, writeNode);
        }

        // uint32_t - stride
        if (data.stride != 0) {
            yaml_write_uint32_t("stride", data.stride, writeNode);
        }

        // VkVertexInputRate - inputRate
        yaml_write_VkEnum("VkVertexInputRate", "input_rate", data.inputRate, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkVertexInputAttributeDescription(std::string const &nodeName,
                                                 YAML::Node const &node,
                                                 VkVertexInputAttributeDescription &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkVertexInputAttributeDescription newData;
    memset(&newData, 0, sizeof(VkVertexInputAttributeDescription));
    try {
        // uint32_t - location
        yaml_read_uint32_t("location", readNode, newData.location);

        // uint32_t - binding
        yaml_read_uint32_t("binding", readNode, newData.binding);

        // VkFormat - format
        yaml_read_VkEnum("VkFormat", "format", readNode, newData.format);

        // uint32_t - offset
        yaml_read_uint32_t("offset", readNode, newData.offset);
    } catch (foeYamlException const &e) {
        cleanup_VkVertexInputAttributeDescription(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkVertexInputAttributeDescription(std::string const &nodeName,
                                                  VkVertexInputAttributeDescription const &data,
                                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // uint32_t - location
        if (data.location != 0) {
            yaml_write_uint32_t("location", data.location, writeNode);
        }

        // uint32_t - binding
        if (data.binding != 0) {
            yaml_write_uint32_t("binding", data.binding, writeNode);
        }

        // VkFormat - format
        yaml_write_VkEnum("VkFormat", "format", data.format, writeNode);

        // uint32_t - offset
        if (data.offset != 0) {
            yaml_write_uint32_t("offset", data.offset, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineVertexInputStateCreateInfo(std::string const &nodeName,
                                                    YAML::Node const &node,
                                                    VkPipelineVertexInputStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineVertexInputStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // VkPipelineVertexInputStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineVertexInputStateCreateFlags", "flags", readNode, newData.flags);

        // VkVertexInputBindingDescription* -
        // pVertexBindingDescriptions[vertexBindingDescriptionCount]
        if (YAML::Node pVertexBindingDescriptionsNode = readNode["vertex_binding_descriptions"];
            pVertexBindingDescriptionsNode) {
            newData.vertexBindingDescriptionCount = pVertexBindingDescriptionsNode.size();
            if (newData.vertexBindingDescriptionCount > 0) {
                newData.pVertexBindingDescriptions = (VkVertexInputBindingDescription *)malloc(
                    newData.vertexBindingDescriptionCount *
                    sizeof(VkVertexInputBindingDescription));
                for (size_t i = 0; i < newData.vertexBindingDescriptionCount; ++i) {
                    YAML::Node subReadNode = pVertexBindingDescriptionsNode[i];
                    if (!yaml_read_VkVertexInputBindingDescription(
                            "", subReadNode,
                            (VkVertexInputBindingDescription &)
                                newData.pVertexBindingDescriptions[i])) {
                        throw foeYamlException{
                            "vertex_binding_descriptions - Failed to read list-node"};
                    }
                }
            }
        }

        // VkVertexInputAttributeDescription* -
        // pVertexAttributeDescriptions[vertexAttributeDescriptionCount]
        if (YAML::Node pVertexAttributeDescriptionsNode = readNode["vertex_attribute_descriptions"];
            pVertexAttributeDescriptionsNode) {
            newData.vertexAttributeDescriptionCount = pVertexAttributeDescriptionsNode.size();
            if (newData.vertexAttributeDescriptionCount > 0) {
                newData.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription *)malloc(
                    newData.vertexAttributeDescriptionCount *
                    sizeof(VkVertexInputAttributeDescription));
                for (size_t i = 0; i < newData.vertexAttributeDescriptionCount; ++i) {
                    YAML::Node subReadNode = pVertexAttributeDescriptionsNode[i];
                    if (!yaml_read_VkVertexInputAttributeDescription(
                            "", subReadNode,
                            (VkVertexInputAttributeDescription &)
                                newData.pVertexAttributeDescriptions[i])) {
                        throw foeYamlException{
                            "vertex_attribute_descriptions - Failed to read list-node"};
                    }
                }
            }
        }
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineVertexInputStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineVertexInputStateCreateInfo(
    std::string const &nodeName,
    VkPipelineVertexInputStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineVertexInputStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineVertexInputStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // VkVertexInputBindingDescription* -
        // pVertexBindingDescriptions[vertexBindingDescriptionCount]
        if (data.vertexBindingDescriptionCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.vertexBindingDescriptionCount; ++i) {
                YAML::Node newNode;
                yaml_write_VkVertexInputBindingDescription("", data.pVertexBindingDescriptions[i],
                                                           newNode);
                subWriteNode.push_back(newNode);
            }

            writeNode["vertex_binding_descriptions"] = subWriteNode;
        }

        // VkVertexInputAttributeDescription* -
        // pVertexAttributeDescriptions[vertexAttributeDescriptionCount]
        if (data.vertexAttributeDescriptionCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.vertexAttributeDescriptionCount; ++i) {
                YAML::Node newNode;
                yaml_write_VkVertexInputAttributeDescription(
                    "", data.pVertexAttributeDescriptions[i], newNode);
                subWriteNode.push_back(newNode);
            }

            writeNode["vertex_attribute_descriptions"] = subWriteNode;
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineInputAssemblyStateCreateInfo(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineInputAssemblyStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineInputAssemblyStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        // VkPipelineInputAssemblyStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineInputAssemblyStateCreateFlags", "flags", readNode,
                         newData.flags);

        // VkPrimitiveTopology - topology
        yaml_read_VkEnum("VkPrimitiveTopology", "topology", readNode, newData.topology);

        // VkBool32 - primitiveRestartEnable
        yaml_read_VkBool32("primitive_restart_enable", readNode, newData.primitiveRestartEnable);
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineInputAssemblyStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineInputAssemblyStateCreateInfo(
    std::string const &nodeName,
    VkPipelineInputAssemblyStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineInputAssemblyStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineInputAssemblyStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // VkPrimitiveTopology - topology
        yaml_write_VkEnum("VkPrimitiveTopology", "topology", data.topology, writeNode);

        // VkBool32 - primitiveRestartEnable
        if (data.primitiveRestartEnable != 0) {
            yaml_write_VkBool32("primitive_restart_enable", data.primitiveRestartEnable, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_VkPipelineTessellationStateCreateInfo(std::string const &nodeName,
                                                     YAML::Node const &node,
                                                     VkPipelineTessellationStateCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    VkPipelineTessellationStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineTessellationStateCreateInfo));
    try {
        // VkStructureType - sType
        newData.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

        // VkPipelineTessellationStateCreateFlags - flags
        yaml_read_VkEnum("VkPipelineTessellationStateCreateFlags", "flags", readNode,
                         newData.flags);

        // uint32_t - patchControlPoints
        yaml_read_uint32_t("patch_control_points", readNode, newData.patchControlPoints);
    } catch (foeYamlException const &e) {
        cleanup_VkPipelineTessellationStateCreateInfo(&newData);
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_VkPipelineTessellationStateCreateInfo(
    std::string const &nodeName,
    VkPipelineTessellationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // VkStructureType - sType
        // Do Nothing

        // VkPipelineTessellationStateCreateFlags - flags
        if (data.flags != 0) {
            yaml_write_VkEnum("VkPipelineTessellationStateCreateFlags", "flags", data.flags,
                              writeNode);
        }

        // uint32_t - patchControlPoints
        if (data.patchControlPoints != 0) {
            yaml_write_uint32_t("patch_control_points", data.patchControlPoints, writeNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException{nodeName + "::" + e.what()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}
