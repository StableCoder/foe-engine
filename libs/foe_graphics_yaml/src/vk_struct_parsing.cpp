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

#include <foe/graphics/yaml/vk_type_parsing.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vulkan/vulkan.h>

#include <string>

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_required<VkStencilOpState>(std::string const &nodeName,
                                                              YAML::Node const &node,
                                                              VkStencilOpState &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName +
                               " - Required node not found to parse as 'VkStencilOpState'");
    }

    bool read = false;
    try {
        read |= yaml_read_optional<VkStencilOp>("failOp", subNode, data.failOp);
        read |= yaml_read_optional<VkStencilOp>("passOp", subNode, data.passOp);
        read |= yaml_read_optional<VkStencilOp>("depthFailOp", subNode, data.depthFailOp);
        read |= yaml_read_optional<VkCompareOp>("compareOp", subNode, data.compareOp);
        read |= yaml_read_optional<uint32_t>("compareMask", subNode, data.compareMask);
        read |= yaml_read_optional<uint32_t>("writeMask", subNode, data.writeMask);
        read |= yaml_read_optional<uint32_t>("reference", subNode, data.reference);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkStencilOpState>(std::string const &nodeName,
                                                              YAML::Node const &node,
                                                              VkStencilOpState &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        read |= yaml_read_optional<VkStencilOp>("failOp", subNode, data.failOp);
        read |= yaml_read_optional<VkStencilOp>("passOp", subNode, data.passOp);
        read |= yaml_read_optional<VkStencilOp>("depthFailOp", subNode, data.depthFailOp);
        read |= yaml_read_optional<VkCompareOp>("compareOp", subNode, data.compareOp);
        read |= yaml_read_optional<uint32_t>("compareMask", subNode, data.compareMask);
        read |= yaml_read_optional<uint32_t>("writeMask", subNode, data.writeMask);
        read |= yaml_read_optional<uint32_t>("reference", subNode, data.reference);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_required<VkStencilOpState>(std::string const &nodeName,
                                                               VkStencilOpState const &data,
                                                               YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required<VkStencilOp>("failOp", data.failOp, writeNode);
        yaml_write_required<VkStencilOp>("passOp", data.passOp, writeNode);
        yaml_write_required<VkStencilOp>("depthFailOp", data.depthFailOp, writeNode);
        yaml_write_required<VkCompareOp>("compareOp", data.compareOp, writeNode);
        yaml_write_required<uint32_t>("compareMask", data.compareMask, writeNode);
        yaml_write_required<uint32_t>("writeMask", data.writeMask, writeNode);
        yaml_write_required<uint32_t>("reference", data.reference, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkStencilOpState>(std::string const &nodeName,
                                                               VkStencilOpState const &defaultData,
                                                               VkStencilOpState const &data,
                                                               YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        addedNode |=
            yaml_write_optional<VkStencilOp>("failOp", defaultData.failOp, data.failOp, writeNode);
        addedNode |=
            yaml_write_optional<VkStencilOp>("passOp", defaultData.passOp, data.passOp, writeNode);
        addedNode |= yaml_write_optional<VkStencilOp>("depthFailOp", defaultData.depthFailOp,
                                                      data.depthFailOp, writeNode);
        addedNode |= yaml_write_optional<VkCompareOp>("compareOp", defaultData.compareOp,
                                                      data.compareOp, writeNode);
        addedNode |= yaml_write_optional<uint32_t>("compareMask", defaultData.compareMask,
                                                   data.compareMask, writeNode);
        addedNode |= yaml_write_optional<uint32_t>("writeMask", defaultData.writeMask,
                                                   data.writeMask, writeNode);
        addedNode |= yaml_write_optional<uint32_t>("reference", defaultData.reference,
                                                   data.reference, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (addedNode) {
        if (nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_required<VkPipelineRasterizationStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineRasterizationStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineRasterizationStateCreateInfo'");
    }

    bool read = false;
    try {
        read |= yaml_read_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", subNode, data.flags);
        read |= yaml_read_optional<VkBool32>("depthClampEnable", subNode, data.depthClampEnable);
        read |= yaml_read_optional<VkBool32>("rasterizerDiscardEnable", subNode,
                                             data.rasterizerDiscardEnable);
        read |= yaml_read_optional<VkPolygonMode>("polygonMode", subNode, data.polygonMode);
        read |= yaml_read_optional_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", subNode,
                                                       data.cullMode);
        read |= yaml_read_optional<VkFrontFace>("frontFace", subNode, data.frontFace);
        read |= yaml_read_optional<VkBool32>("depthBiasEnable", subNode, data.depthBiasEnable);
        read |= yaml_read_optional<float>("depthBiasConstantFactor", subNode,
                                          data.depthBiasConstantFactor);
        read |= yaml_read_optional<float>("depthBiasClamp", subNode, data.depthBiasClamp);
        read |=
            yaml_read_optional<float>("depthBiasSlopeFactor", subNode, data.depthBiasSlopeFactor);
        read |= yaml_read_optional<float>("lineWidth", subNode, data.lineWidth);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineRasterizationStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineRasterizationStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        read |= yaml_read_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", subNode, data.flags);
        read |= yaml_read_optional<VkBool32>("depthClampEnable", subNode, data.depthClampEnable);
        read |= yaml_read_optional<VkBool32>("rasterizerDiscardEnable", subNode,
                                             data.rasterizerDiscardEnable);
        read |= yaml_read_optional<VkPolygonMode>("polygonMode", subNode, data.polygonMode);
        read |= yaml_read_optional_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", subNode,
                                                       data.cullMode);
        read |= yaml_read_optional<VkFrontFace>("frontFace", subNode, data.frontFace);
        read |= yaml_read_optional<VkBool32>("depthBiasEnable", subNode, data.depthBiasEnable);
        read |= yaml_read_optional<float>("depthBiasConstantFactor", subNode,
                                          data.depthBiasConstantFactor);
        read |= yaml_read_optional<float>("depthBiasClamp", subNode, data.depthBiasClamp);
        read |=
            yaml_read_optional<float>("depthBiasSlopeFactor", subNode, data.depthBiasSlopeFactor);
        read |= yaml_read_optional<float>("lineWidth", subNode, data.lineWidth);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_required<VkPipelineRasterizationStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineRasterizationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", data.flags, writeNode);
        yaml_write_required<VkBool32>("depthClampEnable", data.depthClampEnable, writeNode);
        yaml_write_required<VkBool32>("rasterizerDiscardEnable", data.rasterizerDiscardEnable,
                                      writeNode);
        yaml_write_required<VkPolygonMode>("polygonMode", data.polygonMode, writeNode);
        yaml_write_required_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", data.cullMode,
                                                writeNode);
        yaml_write_required<VkFrontFace>("frontFace", data.frontFace, writeNode);
        yaml_write_required<VkBool32>("depthBiasEnable", data.depthBiasEnable, writeNode);
        yaml_write_required<float>("depthBiasConstantFactor", data.depthBiasConstantFactor,
                                   writeNode);
        yaml_write_required<float>("depthBiasClamp", data.depthBiasClamp, writeNode);
        yaml_write_required<float>("depthBiasSlopeFactor", data.depthBiasSlopeFactor, writeNode);
        yaml_write_required<float>("lineWidth", data.lineWidth, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineRasterizationStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineRasterizationStateCreateInfo const &defaultData,
    VkPipelineRasterizationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        addedNode |= yaml_write_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);
        addedNode |= yaml_write_optional<VkBool32>("depthClampEnable", defaultData.depthClampEnable,
                                                   data.depthClampEnable, writeNode);
        addedNode |= yaml_write_optional<VkBool32>("rasterizerDiscardEnable",
                                                   defaultData.rasterizerDiscardEnable,
                                                   data.rasterizerDiscardEnable, writeNode);
        addedNode |= yaml_write_optional<VkPolygonMode>("polygonMode", defaultData.polygonMode,
                                                        data.polygonMode, writeNode);
        addedNode |= yaml_write_optional_vk<VkCullModeFlags>(
            "VkCullModeFlags", "cullMode", data.cullMode, defaultData.cullMode, writeNode);
        addedNode |= yaml_write_optional<VkFrontFace>("frontFace", defaultData.frontFace,
                                                      data.frontFace, writeNode);
        addedNode |= yaml_write_optional<VkBool32>("depthBiasEnable", defaultData.depthBiasEnable,
                                                   data.depthBiasEnable, writeNode);
        addedNode |= yaml_write_optional<float>("depthBiasConstantFactor",
                                                defaultData.depthBiasConstantFactor,
                                                data.depthBiasConstantFactor, writeNode);
        addedNode |= yaml_write_optional<float>("depthBiasClamp", defaultData.depthBiasClamp,
                                                data.depthBiasClamp, writeNode);
        addedNode |=
            yaml_write_optional<float>("depthBiasSlopeFactor", defaultData.depthBiasSlopeFactor,
                                       data.depthBiasSlopeFactor, writeNode);
        addedNode |= yaml_write_optional<float>("lineWidth", defaultData.lineWidth, data.lineWidth,
                                                writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (addedNode) {
        if (nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_required<VkPipelineDepthStencilStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineDepthStencilStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineDepthStencilStateCreateInfo'");
    }

    bool read = false;
    try {
        read |= yaml_read_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", subNode, data.flags);
        read |= yaml_read_optional<VkBool32>("depthTestEnable", subNode, data.depthTestEnable);
        read |= yaml_read_optional<VkBool32>("depthWriteEnable", subNode, data.depthWriteEnable);
        read |= yaml_read_optional<VkCompareOp>("depthCompareOp", subNode, data.depthCompareOp);
        read |= yaml_read_optional<VkBool32>("depthBoundsTestEnable", subNode,
                                             data.depthBoundsTestEnable);
        read |= yaml_read_optional<VkBool32>("stencilTestEnable", subNode, data.stencilTestEnable);
        read |= yaml_read_optional<VkStencilOpState>("front", subNode, data.front);
        read |= yaml_read_optional<VkStencilOpState>("back", subNode, data.back);
        read |= yaml_read_optional<float>("minDepthBounds", subNode, data.minDepthBounds);
        read |= yaml_read_optional<float>("maxDepthBounds", subNode, data.maxDepthBounds);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineDepthStencilStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineDepthStencilStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        read |= yaml_read_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", subNode, data.flags);
        read |= yaml_read_optional<VkBool32>("depthTestEnable", subNode, data.depthTestEnable);
        read |= yaml_read_optional<VkBool32>("depthWriteEnable", subNode, data.depthWriteEnable);
        read |= yaml_read_optional<VkCompareOp>("depthCompareOp", subNode, data.depthCompareOp);
        read |= yaml_read_optional<VkBool32>("depthBoundsTestEnable", subNode,
                                             data.depthBoundsTestEnable);
        read |= yaml_read_optional<VkBool32>("stencilTestEnable", subNode, data.stencilTestEnable);
        read |= yaml_read_optional<VkStencilOpState>("front", subNode, data.front);
        read |= yaml_read_optional<VkStencilOpState>("back", subNode, data.back);
        read |= yaml_read_optional<float>("minDepthBounds", subNode, data.minDepthBounds);
        read |= yaml_read_optional<float>("maxDepthBounds", subNode, data.maxDepthBounds);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_required<VkPipelineDepthStencilStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineDepthStencilStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", data.flags, writeNode);
        yaml_write_required<VkBool32>("depthTestEnable", data.depthTestEnable, writeNode);
        yaml_write_required<VkBool32>("depthWriteEnable", data.depthWriteEnable, writeNode);
        yaml_write_required<VkCompareOp>("depthCompareOp", data.depthCompareOp, writeNode);
        yaml_write_required<VkBool32>("depthBoundsTestEnable", data.depthBoundsTestEnable,
                                      writeNode);
        yaml_write_required<VkBool32>("stencilTestEnable", data.stencilTestEnable, writeNode);
        yaml_write_required<VkStencilOpState>("front", data.front, writeNode);
        yaml_write_required<VkStencilOpState>("back", data.back, writeNode);
        yaml_write_required<float>("minDepthBounds", data.minDepthBounds, writeNode);
        yaml_write_required<float>("maxDepthBounds", data.maxDepthBounds, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineDepthStencilStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineDepthStencilStateCreateInfo const &defaultData,
    VkPipelineDepthStencilStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        addedNode |= yaml_write_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);
        addedNode |= yaml_write_optional<VkBool32>("depthTestEnable", defaultData.depthTestEnable,
                                                   data.depthTestEnable, writeNode);
        addedNode |= yaml_write_optional<VkBool32>("depthWriteEnable", defaultData.depthWriteEnable,
                                                   data.depthWriteEnable, writeNode);
        addedNode |= yaml_write_optional<VkCompareOp>("depthCompareOp", defaultData.depthCompareOp,
                                                      data.depthCompareOp, writeNode);
        addedNode |= yaml_write_optional<VkBool32>("depthBoundsTestEnable",
                                                   defaultData.depthBoundsTestEnable,
                                                   data.depthBoundsTestEnable, writeNode);
        addedNode |= yaml_write_optional<VkBool32>(
            "stencilTestEnable", defaultData.stencilTestEnable, data.stencilTestEnable, writeNode);
        addedNode |= yaml_write_optional<VkStencilOpState>("front", defaultData.front, data.front,
                                                           writeNode);
        addedNode |=
            yaml_write_optional<VkStencilOpState>("back", defaultData.back, data.back, writeNode);
        addedNode |= yaml_write_optional<float>("minDepthBounds", defaultData.minDepthBounds,
                                                data.minDepthBounds, writeNode);
        addedNode |= yaml_write_optional<float>("maxDepthBounds", defaultData.maxDepthBounds,
                                                data.maxDepthBounds, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (addedNode) {
        if (nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_required<VkPipelineColorBlendAttachmentState>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineColorBlendAttachmentState &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineColorBlendAttachmentState'");
    }

    bool read = false;
    try {
        read |= yaml_read_optional<VkBool32>("blendEnable", subNode, data.blendEnable);
        read |= yaml_read_optional<VkBlendFactor>("srcColorBlendFactor", subNode,
                                                  data.srcColorBlendFactor);
        read |= yaml_read_optional<VkBlendFactor>("dstColorBlendFactor", subNode,
                                                  data.dstColorBlendFactor);
        read |= yaml_read_optional<VkBlendOp>("colorBlendOp", subNode, data.colorBlendOp);
        read |= yaml_read_optional<VkBlendFactor>("srcAlphaBlendFactor", subNode,
                                                  data.srcAlphaBlendFactor);
        read |= yaml_read_optional<VkBlendFactor>("dstAlphaBlendFactor", subNode,
                                                  data.dstAlphaBlendFactor);
        read |= yaml_read_optional<VkBlendOp>("alphaBlendOp", subNode, data.alphaBlendOp);
        read |= yaml_read_optional_vk<VkColorComponentFlags>(
            "VkColorComponentFlags", "colorWriteMask", subNode, data.colorWriteMask);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineColorBlendAttachmentState>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineColorBlendAttachmentState &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        read |= yaml_read_optional<VkBool32>("blendEnable", subNode, data.blendEnable);
        read |= yaml_read_optional<VkBlendFactor>("srcColorBlendFactor", subNode,
                                                  data.srcColorBlendFactor);
        read |= yaml_read_optional<VkBlendFactor>("dstColorBlendFactor", subNode,
                                                  data.dstColorBlendFactor);
        read |= yaml_read_optional<VkBlendOp>("colorBlendOp", subNode, data.colorBlendOp);
        read |= yaml_read_optional<VkBlendFactor>("srcAlphaBlendFactor", subNode,
                                                  data.srcAlphaBlendFactor);
        read |= yaml_read_optional<VkBlendFactor>("dstAlphaBlendFactor", subNode,
                                                  data.dstAlphaBlendFactor);
        read |= yaml_read_optional<VkBlendOp>("alphaBlendOp", subNode, data.alphaBlendOp);
        read |= yaml_read_optional_vk<VkColorComponentFlags>(
            "VkColorComponentFlags", "colorWriteMask", subNode, data.colorWriteMask);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_required<VkPipelineColorBlendAttachmentState>(
    std::string const &nodeName,
    VkPipelineColorBlendAttachmentState const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required<VkBool32>("blendEnable", data.blendEnable, writeNode);
        yaml_write_required<VkBlendFactor>("srcColorBlendFactor", data.srcColorBlendFactor,
                                           writeNode);
        yaml_write_required<VkBlendFactor>("dstColorBlendFactor", data.dstColorBlendFactor,
                                           writeNode);
        yaml_write_required<VkBlendOp>("colorBlendOp", data.colorBlendOp, writeNode);
        yaml_write_required<VkBlendFactor>("srcAlphaBlendFactor", data.srcAlphaBlendFactor,
                                           writeNode);
        yaml_write_required<VkBlendFactor>("dstAlphaBlendFactor", data.dstAlphaBlendFactor,
                                           writeNode);
        yaml_write_required<VkBlendOp>("alphaBlendOp", data.alphaBlendOp, writeNode);
        yaml_write_required_vk<VkColorComponentFlags>("VkColorComponentFlags", "colorWriteMask",
                                                      data.colorWriteMask, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineColorBlendAttachmentState>(
    std::string const &nodeName,
    VkPipelineColorBlendAttachmentState const &defaultData,
    VkPipelineColorBlendAttachmentState const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        addedNode |= yaml_write_optional<VkBool32>("blendEnable", defaultData.blendEnable,
                                                   data.blendEnable, writeNode);
        addedNode |= yaml_write_optional<VkBlendFactor>("srcColorBlendFactor",
                                                        defaultData.srcColorBlendFactor,
                                                        data.srcColorBlendFactor, writeNode);
        addedNode |= yaml_write_optional<VkBlendFactor>("dstColorBlendFactor",
                                                        defaultData.dstColorBlendFactor,
                                                        data.dstColorBlendFactor, writeNode);
        addedNode |= yaml_write_optional<VkBlendOp>("colorBlendOp", defaultData.colorBlendOp,
                                                    data.colorBlendOp, writeNode);
        addedNode |= yaml_write_optional<VkBlendFactor>("srcAlphaBlendFactor",
                                                        defaultData.srcAlphaBlendFactor,
                                                        data.srcAlphaBlendFactor, writeNode);
        addedNode |= yaml_write_optional<VkBlendFactor>("dstAlphaBlendFactor",
                                                        defaultData.dstAlphaBlendFactor,
                                                        data.dstAlphaBlendFactor, writeNode);
        addedNode |= yaml_write_optional<VkBlendOp>("alphaBlendOp", defaultData.alphaBlendOp,
                                                    data.alphaBlendOp, writeNode);
        addedNode |= yaml_write_optional_vk<VkColorComponentFlags>(
            "VkColorComponentFlags", "colorWriteMask", data.colorWriteMask,
            defaultData.colorWriteMask, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (addedNode) {
        if (nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}
