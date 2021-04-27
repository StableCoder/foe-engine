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

#include <cstring>
#include <string>

template <>
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPushConstantRange>(std::string const &nodeName,
                                                                 YAML::Node const &node,
                                                                 VkPushConstantRange &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName +
                               " - Required node not found to parse as 'VkPushConstantRange'");
    }

    bool read = false;
    try {
        // stageFlags
        read |= yaml_read_optional_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                          subNode, data.stageFlags);

        // offset
        read |= yaml_read_optional<uint32_t>("offset", subNode, data.offset);

        // size
        read |= yaml_read_optional<uint32_t>("size", subNode, data.size);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPushConstantRange>(std::string const &nodeName,
                                                                 YAML::Node const &node,
                                                                 VkPushConstantRange &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // stageFlags
        read |= yaml_read_optional_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                          subNode, data.stageFlags);

        // offset
        read |= yaml_read_optional<uint32_t>("offset", subNode, data.offset);

        // size
        read |= yaml_read_optional<uint32_t>("size", subNode, data.size);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPushConstantRange>(std::string const &nodeName,
                                                                  VkPushConstantRange const &data,
                                                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // stageFlags
        yaml_write_required_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                   data.stageFlags, writeNode);

        // offset
        yaml_write_required<uint32_t>("offset", data.offset, writeNode);

        // size
        yaml_write_required<uint32_t>("size", data.size, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPushConstantRange>(
    std::string const &nodeName,
    VkPushConstantRange const &defaultData,
    VkPushConstantRange const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // stageFlags
        addedNode |= yaml_write_optional_vk<VkShaderStageFlags>(
            "VkShaderStageFlags", "stageFlags", data.stageFlags, defaultData.stageFlags, writeNode);

        // offset
        addedNode |=
            yaml_write_optional<uint32_t>("offset", defaultData.offset, data.offset, writeNode);

        // size
        addedNode |= yaml_write_optional<uint32_t>("size", defaultData.size, data.size, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkDescriptorSetLayoutBinding>(
    std::string const &nodeName, YAML::Node const &node, VkDescriptorSetLayoutBinding &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName + " - Required node not found to parse as 'VkDescriptorSetLayoutBinding'");
    }

    bool read = false;
    try {
        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // descriptorType
        read |=
            yaml_read_optional<VkDescriptorType>("descriptorType", subNode, data.descriptorType);

        // descriptorCount
        read |= yaml_read_optional<uint32_t>("descriptorCount", subNode, data.descriptorCount);

        // stageFlags
        read |= yaml_read_optional_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                          subNode, data.stageFlags);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkDescriptorSetLayoutBinding>(
    std::string const &nodeName, YAML::Node const &node, VkDescriptorSetLayoutBinding &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // descriptorType
        read |=
            yaml_read_optional<VkDescriptorType>("descriptorType", subNode, data.descriptorType);

        // descriptorCount
        read |= yaml_read_optional<uint32_t>("descriptorCount", subNode, data.descriptorCount);

        // stageFlags
        read |= yaml_read_optional_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                          subNode, data.stageFlags);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkDescriptorSetLayoutBinding>(
    std::string const &nodeName, VkDescriptorSetLayoutBinding const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // binding
        yaml_write_required<uint32_t>("binding", data.binding, writeNode);

        // descriptorType
        yaml_write_required<VkDescriptorType>("descriptorType", data.descriptorType, writeNode);

        // descriptorCount
        yaml_write_required<uint32_t>("descriptorCount", data.descriptorCount, writeNode);

        // stageFlags
        yaml_write_required_vk<VkShaderStageFlags>("VkShaderStageFlags", "stageFlags",
                                                   data.stageFlags, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkDescriptorSetLayoutBinding>(
    std::string const &nodeName,
    VkDescriptorSetLayoutBinding const &defaultData,
    VkDescriptorSetLayoutBinding const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // binding
        addedNode |=
            yaml_write_optional<uint32_t>("binding", defaultData.binding, data.binding, writeNode);

        // descriptorType
        addedNode |= yaml_write_optional<VkDescriptorType>(
            "descriptorType", defaultData.descriptorType, data.descriptorType, writeNode);

        // descriptorCount
        addedNode |= yaml_write_optional<uint32_t>("descriptorCount", defaultData.descriptorCount,
                                                   data.descriptorCount, writeNode);

        // stageFlags
        addedNode |= yaml_write_optional_vk<VkShaderStageFlags>(
            "VkShaderStageFlags", "stageFlags", data.stageFlags, defaultData.stageFlags, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkDescriptorSetLayoutCreateInfo>(
    std::string const &nodeName, YAML::Node const &node, VkDescriptorSetLayoutCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName + " - Required node not found to parse as 'VkDescriptorSetLayoutCreateInfo'");
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkDescriptorSetLayoutCreateFlags>(
            "VkDescriptorSetLayoutCreateFlags", "flags", subNode, data.flags);

        // pBindings / bindingCount
        std::unique_ptr<VkDescriptorSetLayoutBinding[]> pBindings;
        if (auto bindingsNode = subNode["bindings"]; bindingsNode) {
            pBindings.reset(new VkDescriptorSetLayoutBinding[bindingsNode.size()]);
            memset(pBindings.get(), 0, sizeof(VkDescriptorSetLayoutBinding) * bindingsNode.size());
            size_t count = 0;
            for (auto it = bindingsNode.begin(); it != bindingsNode.end(); ++it) {
                yaml_read_required("", *it, pBindings[count]);
                ++count;
            }
            data.bindingCount = bindingsNode.size();
            read = true;
        } else {
            throw foeYamlException{"bindings - Required node not found"};
        }

        // Releasing pointer members
        data.pBindings = pBindings.release();
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkDescriptorSetLayoutCreateInfo>(
    std::string const &nodeName, YAML::Node const &node, VkDescriptorSetLayoutCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkDescriptorSetLayoutCreateFlags>(
            "VkDescriptorSetLayoutCreateFlags", "flags", subNode, data.flags);

        // pBindings / bindingCount
        std::unique_ptr<VkDescriptorSetLayoutBinding[]> pBindings;
        if (auto bindingsNode = subNode["bindings"]; bindingsNode) {
            pBindings.reset(new VkDescriptorSetLayoutBinding[bindingsNode.size()]);
            memset(pBindings.get(), 0, sizeof(VkDescriptorSetLayoutBinding) * bindingsNode.size());
            size_t count = 0;
            for (auto it = bindingsNode.begin(); it != bindingsNode.end(); ++it) {
                yaml_read_required("", *it, pBindings[count]);
                ++count;
            }
            data.bindingCount = bindingsNode.size();
            read = true;
        }

        // Releasing pointer members
        data.pBindings = pBindings.release();
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkDescriptorSetLayoutCreateInfo>(
    std::string const &nodeName, VkDescriptorSetLayoutCreateInfo const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkDescriptorSetLayoutCreateFlags>("VkDescriptorSetLayoutCreateFlags",
                                                                 "flags", data.flags, writeNode);

        // pBindings / bindingCount
        YAML::Node bindingsNode;
        for (uint32_t i = 0; i < data.bindingCount; ++i) {
            YAML::Node newNode;
            yaml_write_required<VkDescriptorSetLayoutBinding>("", data.pBindings[i], newNode);
            bindingsNode.push_back(newNode);
        }
        writeNode["bindings"] = bindingsNode;

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkDescriptorSetLayoutCreateInfo>(
    std::string const &nodeName,
    VkDescriptorSetLayoutCreateInfo const &defaultData,
    VkDescriptorSetLayoutCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // flags
        addedNode |= yaml_write_optional_vk<VkDescriptorSetLayoutCreateFlags>(
            "VkDescriptorSetLayoutCreateFlags", "flags", data.flags, defaultData.flags, writeNode);

        // pBindings / bindingCount
        if (data.bindingCount > 0) {
            YAML::Node bindingsNode;
            for (uint32_t i = 0; i < data.bindingCount; ++i) {
                YAML::Node newNode;
                yaml_write_required<VkDescriptorSetLayoutBinding>("", data.pBindings[i], newNode);
                bindingsNode.push_back(newNode);
            }
            writeNode["bindings"] = bindingsNode;
            addedNode = true;
        }

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkStencilOpState>(std::string const &nodeName,
                                                              YAML::Node const &node,
                                                              VkStencilOpState &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName +
                               " - Required node not found to parse as 'VkStencilOpState'");
    }

    bool read = false;
    try {
        // failOp
        read |= yaml_read_optional<VkStencilOp>("failOp", subNode, data.failOp);

        // passOp
        read |= yaml_read_optional<VkStencilOp>("passOp", subNode, data.passOp);

        // depthFailOp
        read |= yaml_read_optional<VkStencilOp>("depthFailOp", subNode, data.depthFailOp);

        // compareOp
        read |= yaml_read_optional<VkCompareOp>("compareOp", subNode, data.compareOp);

        // compareMask
        read |= yaml_read_optional<uint32_t>("compareMask", subNode, data.compareMask);

        // writeMask
        read |= yaml_read_optional<uint32_t>("writeMask", subNode, data.writeMask);

        // reference
        read |= yaml_read_optional<uint32_t>("reference", subNode, data.reference);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
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
        // failOp
        read |= yaml_read_optional<VkStencilOp>("failOp", subNode, data.failOp);

        // passOp
        read |= yaml_read_optional<VkStencilOp>("passOp", subNode, data.passOp);

        // depthFailOp
        read |= yaml_read_optional<VkStencilOp>("depthFailOp", subNode, data.depthFailOp);

        // compareOp
        read |= yaml_read_optional<VkCompareOp>("compareOp", subNode, data.compareOp);

        // compareMask
        read |= yaml_read_optional<uint32_t>("compareMask", subNode, data.compareMask);

        // writeMask
        read |= yaml_read_optional<uint32_t>("writeMask", subNode, data.writeMask);

        // reference
        read |= yaml_read_optional<uint32_t>("reference", subNode, data.reference);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkStencilOpState>(std::string const &nodeName,
                                                               VkStencilOpState const &data,
                                                               YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // failOp
        yaml_write_required<VkStencilOp>("failOp", data.failOp, writeNode);

        // passOp
        yaml_write_required<VkStencilOp>("passOp", data.passOp, writeNode);

        // depthFailOp
        yaml_write_required<VkStencilOp>("depthFailOp", data.depthFailOp, writeNode);

        // compareOp
        yaml_write_required<VkCompareOp>("compareOp", data.compareOp, writeNode);

        // compareMask
        yaml_write_required<uint32_t>("compareMask", data.compareMask, writeNode);

        // writeMask
        yaml_write_required<uint32_t>("writeMask", data.writeMask, writeNode);

        // reference
        yaml_write_required<uint32_t>("reference", data.reference, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkStencilOpState>(std::string const &nodeName,
                                                               VkStencilOpState const &defaultData,
                                                               VkStencilOpState const &data,
                                                               YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // failOp
        addedNode |=
            yaml_write_optional<VkStencilOp>("failOp", defaultData.failOp, data.failOp, writeNode);

        // passOp
        addedNode |=
            yaml_write_optional<VkStencilOp>("passOp", defaultData.passOp, data.passOp, writeNode);

        // depthFailOp
        addedNode |= yaml_write_optional<VkStencilOp>("depthFailOp", defaultData.depthFailOp,
                                                      data.depthFailOp, writeNode);

        // compareOp
        addedNode |= yaml_write_optional<VkCompareOp>("compareOp", defaultData.compareOp,
                                                      data.compareOp, writeNode);

        // compareMask
        addedNode |= yaml_write_optional<uint32_t>("compareMask", defaultData.compareMask,
                                                   data.compareMask, writeNode);

        // writeMask
        addedNode |= yaml_write_optional<uint32_t>("writeMask", defaultData.writeMask,
                                                   data.writeMask, writeNode);

        // reference
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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineRasterizationStateCreateInfo>(
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
        // flags
        read |= yaml_read_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", subNode, data.flags);

        // depthClampEnable
        read |= yaml_read_optional<VkBool32>("depthClampEnable", subNode, data.depthClampEnable);

        // rasterizerDiscardEnable
        read |= yaml_read_optional<VkBool32>("rasterizerDiscardEnable", subNode,
                                             data.rasterizerDiscardEnable);

        // polygonMode
        read |= yaml_read_optional<VkPolygonMode>("polygonMode", subNode, data.polygonMode);

        // cullMode
        read |= yaml_read_optional_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", subNode,
                                                       data.cullMode);

        // frontFace
        read |= yaml_read_optional<VkFrontFace>("frontFace", subNode, data.frontFace);

        // depthBiasEnable
        read |= yaml_read_optional<VkBool32>("depthBiasEnable", subNode, data.depthBiasEnable);

        // depthBiasConstantFactor
        read |= yaml_read_optional<float>("depthBiasConstantFactor", subNode,
                                          data.depthBiasConstantFactor);

        // depthBiasClamp
        read |= yaml_read_optional<float>("depthBiasClamp", subNode, data.depthBiasClamp);

        // depthBiasSlopeFactor
        read |=
            yaml_read_optional<float>("depthBiasSlopeFactor", subNode, data.depthBiasSlopeFactor);

        // lineWidth
        read |= yaml_read_optional<float>("lineWidth", subNode, data.lineWidth);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
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
        // flags
        read |= yaml_read_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", subNode, data.flags);

        // depthClampEnable
        read |= yaml_read_optional<VkBool32>("depthClampEnable", subNode, data.depthClampEnable);

        // rasterizerDiscardEnable
        read |= yaml_read_optional<VkBool32>("rasterizerDiscardEnable", subNode,
                                             data.rasterizerDiscardEnable);

        // polygonMode
        read |= yaml_read_optional<VkPolygonMode>("polygonMode", subNode, data.polygonMode);

        // cullMode
        read |= yaml_read_optional_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", subNode,
                                                       data.cullMode);

        // frontFace
        read |= yaml_read_optional<VkFrontFace>("frontFace", subNode, data.frontFace);

        // depthBiasEnable
        read |= yaml_read_optional<VkBool32>("depthBiasEnable", subNode, data.depthBiasEnable);

        // depthBiasConstantFactor
        read |= yaml_read_optional<float>("depthBiasConstantFactor", subNode,
                                          data.depthBiasConstantFactor);

        // depthBiasClamp
        read |= yaml_read_optional<float>("depthBiasClamp", subNode, data.depthBiasClamp);

        // depthBiasSlopeFactor
        read |=
            yaml_read_optional<float>("depthBiasSlopeFactor", subNode, data.depthBiasSlopeFactor);

        // lineWidth
        read |= yaml_read_optional<float>("lineWidth", subNode, data.lineWidth);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineRasterizationStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineRasterizationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", data.flags, writeNode);

        // depthClampEnable
        yaml_write_required<VkBool32>("depthClampEnable", data.depthClampEnable, writeNode);

        // rasterizerDiscardEnable
        yaml_write_required<VkBool32>("rasterizerDiscardEnable", data.rasterizerDiscardEnable,
                                      writeNode);

        // polygonMode
        yaml_write_required<VkPolygonMode>("polygonMode", data.polygonMode, writeNode);

        // cullMode
        yaml_write_required_vk<VkCullModeFlags>("VkCullModeFlags", "cullMode", data.cullMode,
                                                writeNode);

        // frontFace
        yaml_write_required<VkFrontFace>("frontFace", data.frontFace, writeNode);

        // depthBiasEnable
        yaml_write_required<VkBool32>("depthBiasEnable", data.depthBiasEnable, writeNode);

        // depthBiasConstantFactor
        yaml_write_required<float>("depthBiasConstantFactor", data.depthBiasConstantFactor,
                                   writeNode);

        // depthBiasClamp
        yaml_write_required<float>("depthBiasClamp", data.depthBiasClamp, writeNode);

        // depthBiasSlopeFactor
        yaml_write_required<float>("depthBiasSlopeFactor", data.depthBiasSlopeFactor, writeNode);

        // lineWidth
        yaml_write_required<float>("lineWidth", data.lineWidth, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
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
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineRasterizationStateCreateFlags>(
            "VkPipelineRasterizationStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);

        // depthClampEnable
        addedNode |= yaml_write_optional<VkBool32>("depthClampEnable", defaultData.depthClampEnable,
                                                   data.depthClampEnable, writeNode);

        // rasterizerDiscardEnable
        addedNode |= yaml_write_optional<VkBool32>("rasterizerDiscardEnable",
                                                   defaultData.rasterizerDiscardEnable,
                                                   data.rasterizerDiscardEnable, writeNode);

        // polygonMode
        addedNode |= yaml_write_optional<VkPolygonMode>("polygonMode", defaultData.polygonMode,
                                                        data.polygonMode, writeNode);

        // cullMode
        addedNode |= yaml_write_optional_vk<VkCullModeFlags>(
            "VkCullModeFlags", "cullMode", data.cullMode, defaultData.cullMode, writeNode);

        // frontFace
        addedNode |= yaml_write_optional<VkFrontFace>("frontFace", defaultData.frontFace,
                                                      data.frontFace, writeNode);

        // depthBiasEnable
        addedNode |= yaml_write_optional<VkBool32>("depthBiasEnable", defaultData.depthBiasEnable,
                                                   data.depthBiasEnable, writeNode);

        // depthBiasConstantFactor
        addedNode |= yaml_write_optional<float>("depthBiasConstantFactor",
                                                defaultData.depthBiasConstantFactor,
                                                data.depthBiasConstantFactor, writeNode);

        // depthBiasClamp
        addedNode |= yaml_write_optional<float>("depthBiasClamp", defaultData.depthBiasClamp,
                                                data.depthBiasClamp, writeNode);

        // depthBiasSlopeFactor
        addedNode |=
            yaml_write_optional<float>("depthBiasSlopeFactor", defaultData.depthBiasSlopeFactor,
                                       data.depthBiasSlopeFactor, writeNode);

        // lineWidth
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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineDepthStencilStateCreateInfo>(
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
        // flags
        read |= yaml_read_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", subNode, data.flags);

        // depthTestEnable
        read |= yaml_read_optional<VkBool32>("depthTestEnable", subNode, data.depthTestEnable);

        // depthWriteEnable
        read |= yaml_read_optional<VkBool32>("depthWriteEnable", subNode, data.depthWriteEnable);

        // depthCompareOp
        read |= yaml_read_optional<VkCompareOp>("depthCompareOp", subNode, data.depthCompareOp);

        // depthBoundsTestEnable
        read |= yaml_read_optional<VkBool32>("depthBoundsTestEnable", subNode,
                                             data.depthBoundsTestEnable);

        // stencilTestEnable
        read |= yaml_read_optional<VkBool32>("stencilTestEnable", subNode, data.stencilTestEnable);

        // front
        read |= yaml_read_optional<VkStencilOpState>("front", subNode, data.front);

        // back
        read |= yaml_read_optional<VkStencilOpState>("back", subNode, data.back);

        // minDepthBounds
        read |= yaml_read_optional<float>("minDepthBounds", subNode, data.minDepthBounds);

        // maxDepthBounds
        read |= yaml_read_optional<float>("maxDepthBounds", subNode, data.maxDepthBounds);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
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
        // flags
        read |= yaml_read_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", subNode, data.flags);

        // depthTestEnable
        read |= yaml_read_optional<VkBool32>("depthTestEnable", subNode, data.depthTestEnable);

        // depthWriteEnable
        read |= yaml_read_optional<VkBool32>("depthWriteEnable", subNode, data.depthWriteEnable);

        // depthCompareOp
        read |= yaml_read_optional<VkCompareOp>("depthCompareOp", subNode, data.depthCompareOp);

        // depthBoundsTestEnable
        read |= yaml_read_optional<VkBool32>("depthBoundsTestEnable", subNode,
                                             data.depthBoundsTestEnable);

        // stencilTestEnable
        read |= yaml_read_optional<VkBool32>("stencilTestEnable", subNode, data.stencilTestEnable);

        // front
        read |= yaml_read_optional<VkStencilOpState>("front", subNode, data.front);

        // back
        read |= yaml_read_optional<VkStencilOpState>("back", subNode, data.back);

        // minDepthBounds
        read |= yaml_read_optional<float>("minDepthBounds", subNode, data.minDepthBounds);

        // maxDepthBounds
        read |= yaml_read_optional<float>("maxDepthBounds", subNode, data.maxDepthBounds);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineDepthStencilStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineDepthStencilStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", data.flags, writeNode);

        // depthTestEnable
        yaml_write_required<VkBool32>("depthTestEnable", data.depthTestEnable, writeNode);

        // depthWriteEnable
        yaml_write_required<VkBool32>("depthWriteEnable", data.depthWriteEnable, writeNode);

        // depthCompareOp
        yaml_write_required<VkCompareOp>("depthCompareOp", data.depthCompareOp, writeNode);

        // depthBoundsTestEnable
        yaml_write_required<VkBool32>("depthBoundsTestEnable", data.depthBoundsTestEnable,
                                      writeNode);

        // stencilTestEnable
        yaml_write_required<VkBool32>("stencilTestEnable", data.stencilTestEnable, writeNode);

        // front
        yaml_write_required<VkStencilOpState>("front", data.front, writeNode);

        // back
        yaml_write_required<VkStencilOpState>("back", data.back, writeNode);

        // minDepthBounds
        yaml_write_required<float>("minDepthBounds", data.minDepthBounds, writeNode);

        // maxDepthBounds
        yaml_write_required<float>("maxDepthBounds", data.maxDepthBounds, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
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
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineDepthStencilStateCreateFlags>(
            "VkPipelineDepthStencilStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);

        // depthTestEnable
        addedNode |= yaml_write_optional<VkBool32>("depthTestEnable", defaultData.depthTestEnable,
                                                   data.depthTestEnable, writeNode);

        // depthWriteEnable
        addedNode |= yaml_write_optional<VkBool32>("depthWriteEnable", defaultData.depthWriteEnable,
                                                   data.depthWriteEnable, writeNode);

        // depthCompareOp
        addedNode |= yaml_write_optional<VkCompareOp>("depthCompareOp", defaultData.depthCompareOp,
                                                      data.depthCompareOp, writeNode);

        // depthBoundsTestEnable
        addedNode |= yaml_write_optional<VkBool32>("depthBoundsTestEnable",
                                                   defaultData.depthBoundsTestEnable,
                                                   data.depthBoundsTestEnable, writeNode);

        // stencilTestEnable
        addedNode |= yaml_write_optional<VkBool32>(
            "stencilTestEnable", defaultData.stencilTestEnable, data.stencilTestEnable, writeNode);

        // front
        addedNode |= yaml_write_optional<VkStencilOpState>("front", defaultData.front, data.front,
                                                           writeNode);

        // back
        addedNode |=
            yaml_write_optional<VkStencilOpState>("back", defaultData.back, data.back, writeNode);

        // minDepthBounds
        addedNode |= yaml_write_optional<float>("minDepthBounds", defaultData.minDepthBounds,
                                                data.minDepthBounds, writeNode);

        // maxDepthBounds
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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineColorBlendAttachmentState>(
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
        // blendEnable
        read |= yaml_read_optional<VkBool32>("blendEnable", subNode, data.blendEnable);

        // srcColorBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("srcColorBlendFactor", subNode,
                                                  data.srcColorBlendFactor);

        // dstColorBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("dstColorBlendFactor", subNode,
                                                  data.dstColorBlendFactor);

        // colorBlendOp
        read |= yaml_read_optional<VkBlendOp>("colorBlendOp", subNode, data.colorBlendOp);

        // srcAlphaBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("srcAlphaBlendFactor", subNode,
                                                  data.srcAlphaBlendFactor);

        // dstAlphaBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("dstAlphaBlendFactor", subNode,
                                                  data.dstAlphaBlendFactor);

        // alphaBlendOp
        read |= yaml_read_optional<VkBlendOp>("alphaBlendOp", subNode, data.alphaBlendOp);

        // colorWriteMask
        read |= yaml_read_optional_vk<VkColorComponentFlags>(
            "VkColorComponentFlags", "colorWriteMask", subNode, data.colorWriteMask);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
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
        // blendEnable
        read |= yaml_read_optional<VkBool32>("blendEnable", subNode, data.blendEnable);

        // srcColorBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("srcColorBlendFactor", subNode,
                                                  data.srcColorBlendFactor);

        // dstColorBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("dstColorBlendFactor", subNode,
                                                  data.dstColorBlendFactor);

        // colorBlendOp
        read |= yaml_read_optional<VkBlendOp>("colorBlendOp", subNode, data.colorBlendOp);

        // srcAlphaBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("srcAlphaBlendFactor", subNode,
                                                  data.srcAlphaBlendFactor);

        // dstAlphaBlendFactor
        read |= yaml_read_optional<VkBlendFactor>("dstAlphaBlendFactor", subNode,
                                                  data.dstAlphaBlendFactor);

        // alphaBlendOp
        read |= yaml_read_optional<VkBlendOp>("alphaBlendOp", subNode, data.alphaBlendOp);

        // colorWriteMask
        read |= yaml_read_optional_vk<VkColorComponentFlags>(
            "VkColorComponentFlags", "colorWriteMask", subNode, data.colorWriteMask);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineColorBlendAttachmentState>(
    std::string const &nodeName,
    VkPipelineColorBlendAttachmentState const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // blendEnable
        yaml_write_required<VkBool32>("blendEnable", data.blendEnable, writeNode);

        // srcColorBlendFactor
        yaml_write_required<VkBlendFactor>("srcColorBlendFactor", data.srcColorBlendFactor,
                                           writeNode);

        // dstColorBlendFactor
        yaml_write_required<VkBlendFactor>("dstColorBlendFactor", data.dstColorBlendFactor,
                                           writeNode);

        // colorBlendOp
        yaml_write_required<VkBlendOp>("colorBlendOp", data.colorBlendOp, writeNode);

        // srcAlphaBlendFactor
        yaml_write_required<VkBlendFactor>("srcAlphaBlendFactor", data.srcAlphaBlendFactor,
                                           writeNode);

        // dstAlphaBlendFactor
        yaml_write_required<VkBlendFactor>("dstAlphaBlendFactor", data.dstAlphaBlendFactor,
                                           writeNode);

        // alphaBlendOp
        yaml_write_required<VkBlendOp>("alphaBlendOp", data.alphaBlendOp, writeNode);

        // colorWriteMask
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
        // blendEnable
        addedNode |= yaml_write_optional<VkBool32>("blendEnable", defaultData.blendEnable,
                                                   data.blendEnable, writeNode);

        // srcColorBlendFactor
        addedNode |= yaml_write_optional<VkBlendFactor>("srcColorBlendFactor",
                                                        defaultData.srcColorBlendFactor,
                                                        data.srcColorBlendFactor, writeNode);

        // dstColorBlendFactor
        addedNode |= yaml_write_optional<VkBlendFactor>("dstColorBlendFactor",
                                                        defaultData.dstColorBlendFactor,
                                                        data.dstColorBlendFactor, writeNode);

        // colorBlendOp
        addedNode |= yaml_write_optional<VkBlendOp>("colorBlendOp", defaultData.colorBlendOp,
                                                    data.colorBlendOp, writeNode);

        // srcAlphaBlendFactor
        addedNode |= yaml_write_optional<VkBlendFactor>("srcAlphaBlendFactor",
                                                        defaultData.srcAlphaBlendFactor,
                                                        data.srcAlphaBlendFactor, writeNode);

        // dstAlphaBlendFactor
        addedNode |= yaml_write_optional<VkBlendFactor>("dstAlphaBlendFactor",
                                                        defaultData.dstAlphaBlendFactor,
                                                        data.dstAlphaBlendFactor, writeNode);

        // alphaBlendOp
        addedNode |= yaml_write_optional<VkBlendOp>("alphaBlendOp", defaultData.alphaBlendOp,
                                                    data.alphaBlendOp, writeNode);

        // colorWriteMask
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

template <>
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineColorBlendStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineColorBlendStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineColorBlendStateCreateInfo'");
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineColorBlendStateCreateFlags>(
            "VkPipelineColorBlendStateCreateFlags", "flags", subNode, data.flags);

        // logicOpEnable
        read |= yaml_read_optional<VkBool32>("logicOpEnable", subNode, data.logicOpEnable);

        // logicOp
        read |= yaml_read_optional<VkLogicOp>("logicOp", subNode, data.logicOp);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineColorBlendStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineColorBlendStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineColorBlendStateCreateFlags>(
            "VkPipelineColorBlendStateCreateFlags", "flags", subNode, data.flags);

        // logicOpEnable
        read |= yaml_read_optional<VkBool32>("logicOpEnable", subNode, data.logicOpEnable);

        // logicOp
        read |= yaml_read_optional<VkLogicOp>("logicOp", subNode, data.logicOp);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineColorBlendStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineColorBlendStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineColorBlendStateCreateFlags>(
            "VkPipelineColorBlendStateCreateFlags", "flags", data.flags, writeNode);

        // logicOpEnable
        yaml_write_required<VkBool32>("logicOpEnable", data.logicOpEnable, writeNode);

        // logicOp
        yaml_write_required<VkLogicOp>("logicOp", data.logicOp, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineColorBlendStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineColorBlendStateCreateInfo const &defaultData,
    VkPipelineColorBlendStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineColorBlendStateCreateFlags>(
            "VkPipelineColorBlendStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);

        // logicOpEnable
        addedNode |= yaml_write_optional<VkBool32>("logicOpEnable", defaultData.logicOpEnable,
                                                   data.logicOpEnable, writeNode);

        // logicOp
        addedNode |=
            yaml_write_optional<VkLogicOp>("logicOp", defaultData.logicOp, data.logicOp, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkVertexInputBindingDescription>(
    std::string const &nodeName, YAML::Node const &node, VkVertexInputBindingDescription &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName + " - Required node not found to parse as 'VkVertexInputBindingDescription'");
    }

    bool read = false;
    try {
        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // stride
        read |= yaml_read_optional<uint32_t>("stride", subNode, data.stride);

        // inputRate
        read |= yaml_read_optional<VkVertexInputRate>("inputRate", subNode, data.inputRate);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkVertexInputBindingDescription>(
    std::string const &nodeName, YAML::Node const &node, VkVertexInputBindingDescription &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // stride
        read |= yaml_read_optional<uint32_t>("stride", subNode, data.stride);

        // inputRate
        read |= yaml_read_optional<VkVertexInputRate>("inputRate", subNode, data.inputRate);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkVertexInputBindingDescription>(
    std::string const &nodeName, VkVertexInputBindingDescription const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // binding
        yaml_write_required<uint32_t>("binding", data.binding, writeNode);

        // stride
        yaml_write_required<uint32_t>("stride", data.stride, writeNode);

        // inputRate
        yaml_write_required<VkVertexInputRate>("inputRate", data.inputRate, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkVertexInputBindingDescription>(
    std::string const &nodeName,
    VkVertexInputBindingDescription const &defaultData,
    VkVertexInputBindingDescription const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // binding
        addedNode |=
            yaml_write_optional<uint32_t>("binding", defaultData.binding, data.binding, writeNode);

        // stride
        addedNode |=
            yaml_write_optional<uint32_t>("stride", defaultData.stride, data.stride, writeNode);

        // inputRate
        addedNode |= yaml_write_optional<VkVertexInputRate>("inputRate", defaultData.inputRate,
                                                            data.inputRate, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkVertexInputAttributeDescription>(
    std::string const &nodeName, YAML::Node const &node, VkVertexInputAttributeDescription &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkVertexInputAttributeDescription'");
    }

    bool read = false;
    try {
        // location
        read |= yaml_read_optional<uint32_t>("location", subNode, data.location);

        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // format
        read |= yaml_read_optional<VkFormat>("format", subNode, data.format);

        // offset
        read |= yaml_read_optional<uint32_t>("offset", subNode, data.offset);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkVertexInputAttributeDescription>(
    std::string const &nodeName, YAML::Node const &node, VkVertexInputAttributeDescription &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // location
        read |= yaml_read_optional<uint32_t>("location", subNode, data.location);

        // binding
        read |= yaml_read_optional<uint32_t>("binding", subNode, data.binding);

        // format
        read |= yaml_read_optional<VkFormat>("format", subNode, data.format);

        // offset
        read |= yaml_read_optional<uint32_t>("offset", subNode, data.offset);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkVertexInputAttributeDescription>(
    std::string const &nodeName, VkVertexInputAttributeDescription const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // location
        yaml_write_required<uint32_t>("location", data.location, writeNode);

        // binding
        yaml_write_required<uint32_t>("binding", data.binding, writeNode);

        // format
        yaml_write_required<VkFormat>("format", data.format, writeNode);

        // offset
        yaml_write_required<uint32_t>("offset", data.offset, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkVertexInputAttributeDescription>(
    std::string const &nodeName,
    VkVertexInputAttributeDescription const &defaultData,
    VkVertexInputAttributeDescription const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // location
        addedNode |= yaml_write_optional<uint32_t>("location", defaultData.location, data.location,
                                                   writeNode);

        // binding
        addedNode |=
            yaml_write_optional<uint32_t>("binding", defaultData.binding, data.binding, writeNode);

        // format
        addedNode |=
            yaml_write_optional<VkFormat>("format", defaultData.format, data.format, writeNode);

        // offset
        addedNode |=
            yaml_write_optional<uint32_t>("offset", defaultData.offset, data.offset, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineVertexInputStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineVertexInputStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineVertexInputStateCreateInfo'");
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineVertexInputStateCreateFlags>(
            "VkPipelineVertexInputStateCreateFlags", "flags", subNode, data.flags);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineVertexInputStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineVertexInputStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineVertexInputStateCreateFlags>(
            "VkPipelineVertexInputStateCreateFlags", "flags", subNode, data.flags);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineVertexInputStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineVertexInputStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineVertexInputStateCreateFlags>(
            "VkPipelineVertexInputStateCreateFlags", "flags", data.flags, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineVertexInputStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineVertexInputStateCreateInfo const &defaultData,
    VkPipelineVertexInputStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineVertexInputStateCreateFlags>(
            "VkPipelineVertexInputStateCreateFlags", "flags", data.flags, defaultData.flags,
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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineInputAssemblyStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineInputAssemblyStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineInputAssemblyStateCreateInfo'");
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineInputAssemblyStateCreateFlags>(
            "VkPipelineInputAssemblyStateCreateFlags", "flags", subNode, data.flags);

        // topology
        read |= yaml_read_optional<VkPrimitiveTopology>("topology", subNode, data.topology);

        // primitiveRestartEnable
        read |= yaml_read_optional<VkBool32>("primitiveRestartEnable", subNode,
                                             data.primitiveRestartEnable);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineInputAssemblyStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineInputAssemblyStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineInputAssemblyStateCreateFlags>(
            "VkPipelineInputAssemblyStateCreateFlags", "flags", subNode, data.flags);

        // topology
        read |= yaml_read_optional<VkPrimitiveTopology>("topology", subNode, data.topology);

        // primitiveRestartEnable
        read |= yaml_read_optional<VkBool32>("primitiveRestartEnable", subNode,
                                             data.primitiveRestartEnable);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineInputAssemblyStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineInputAssemblyStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineInputAssemblyStateCreateFlags>(
            "VkPipelineInputAssemblyStateCreateFlags", "flags", data.flags, writeNode);

        // topology
        yaml_write_required<VkPrimitiveTopology>("topology", data.topology, writeNode);

        // primitiveRestartEnable
        yaml_write_required<VkBool32>("primitiveRestartEnable", data.primitiveRestartEnable,
                                      writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineInputAssemblyStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineInputAssemblyStateCreateInfo const &defaultData,
    VkPipelineInputAssemblyStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineInputAssemblyStateCreateFlags>(
            "VkPipelineInputAssemblyStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);

        // topology
        addedNode |= yaml_write_optional<VkPrimitiveTopology>("topology", defaultData.topology,
                                                              data.topology, writeNode);

        // primitiveRestartEnable
        addedNode |= yaml_write_optional<VkBool32>("primitiveRestartEnable",
                                                   defaultData.primitiveRestartEnable,
                                                   data.primitiveRestartEnable, writeNode);

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
FOE_GFX_YAML_EXPORT void yaml_read_required<VkPipelineTessellationStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineTessellationStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(
            nodeName +
            " - Required node not found to parse as 'VkPipelineTessellationStateCreateInfo'");
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineTessellationStateCreateFlags>(
            "VkPipelineTessellationStateCreateFlags", "flags", subNode, data.flags);

        // patchControlPoints
        read |=
            yaml_read_optional<uint32_t>("patchControlPoints", subNode, data.patchControlPoints);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<VkPipelineTessellationStateCreateInfo>(
    std::string const &nodeName,
    YAML::Node const &node,
    VkPipelineTessellationStateCreateInfo &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;
    try {
        // flags
        read |= yaml_read_optional_vk<VkPipelineTessellationStateCreateFlags>(
            "VkPipelineTessellationStateCreateFlags", "flags", subNode, data.flags);

        // patchControlPoints
        read |=
            yaml_read_optional<uint32_t>("patchControlPoints", subNode, data.patchControlPoints);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return read;
}

template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<VkPipelineTessellationStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineTessellationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // flags
        yaml_write_required_vk<VkPipelineTessellationStateCreateFlags>(
            "VkPipelineTessellationStateCreateFlags", "flags", data.flags, writeNode);

        // patchControlPoints
        yaml_write_required<uint32_t>("patchControlPoints", data.patchControlPoints, writeNode);

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<VkPipelineTessellationStateCreateInfo>(
    std::string const &nodeName,
    VkPipelineTessellationStateCreateInfo const &defaultData,
    VkPipelineTessellationStateCreateInfo const &data,
    YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {
        // flags
        addedNode |= yaml_write_optional_vk<VkPipelineTessellationStateCreateFlags>(
            "VkPipelineTessellationStateCreateFlags", "flags", data.flags, defaultData.flags,
            writeNode);

        // patchControlPoints
        addedNode |=
            yaml_write_optional<uint32_t>("patchControlPoints", defaultData.patchControlPoints,
                                          data.patchControlPoints, writeNode);

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
