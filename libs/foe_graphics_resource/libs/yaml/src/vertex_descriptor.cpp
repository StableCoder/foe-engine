// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.hpp>
#include <foe/graphics/vk/yaml/vertex_descriptor.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

bool yaml_read_vertex_descriptor_internal(std::string const &nodeName,
                                          YAML::Node const &node,
                                          foeEcsGroupTranslator groupTranslator,
                                          foeVertexDescriptorCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read{false};

    try {
        // Resources
        if (auto resNode = subNode["resources"]; resNode) {
            read |= yaml_read_id_optional("vertex_shader", resNode, groupTranslator,
                                          createInfo.vertexShader);
            read |= yaml_read_id_optional("tessellation_control_shader", resNode, groupTranslator,
                                          createInfo.tessellationControlShader);
            read |= yaml_read_id_optional("tessellation_evaluation_shader", resNode,
                                          groupTranslator, createInfo.tessellationEvaluationShader);
            read |= yaml_read_id_optional("geometry_shader", resNode, groupTranslator,
                                          createInfo.geometryShader);
        }

        // Graphics Data
        read |= yaml_read_gfx_vertex_descriptor(
            "graphics_data", subNode, createInfo.vertexInputSCI, createInfo.inputBindingCount,
            createInfo.pInputBindings, createInfo.inputAttributeCount, createInfo.pInputAttributes,
            createInfo.inputAssemblySCI, createInfo.tessellationSCI);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
        return false;
    }

    return read;
}

bool yaml_write_vertex_descriptor_internal(std::string const &nodeName,
                                           foeVertexDescriptorCreateInfo const &vertexDescriptor,
                                           YAML::Node &node) {
    YAML::Node writeNode;

    try {
        { // Resources Node
            YAML::Node resNode;

            if (vertexDescriptor.vertexShader != FOE_INVALID_ID) {
                yaml_write_id("vertex_shader", vertexDescriptor.vertexShader, resNode);
            }

            if (vertexDescriptor.tessellationControlShader != FOE_INVALID_ID) {
                yaml_write_id("tessellation_control_shader",
                              vertexDescriptor.tessellationControlShader, resNode);
            }

            if (vertexDescriptor.tessellationEvaluationShader != FOE_INVALID_ID) {
                yaml_write_id("tessellation_evaluation_shader",
                              vertexDescriptor.tessellationEvaluationShader, resNode);
            }

            if (vertexDescriptor.geometryShader != FOE_INVALID_ID) {
                yaml_write_id("geometry_shader", vertexDescriptor.geometryShader, resNode);
            }

            writeNode["resources"] = resNode;
        }
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeVertexDescriptor' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

} // namespace

char const *yaml_vertex_descriptor_key() { return "vertex_descriptor_v1"; }

void yaml_read_vertex_descriptor(YAML::Node const &node,
                                 foeEcsGroupTranslator groupTranslator,
                                 foeResourceCreateInfo *pCreateInfo) {
    foeVertexDescriptorCreateInfo vdCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_vertex_descriptor_internal(yaml_vertex_descriptor_key(), node, groupTranslator, vdCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeVertexDescriptorCreateInfo *)pSrc;
        new (pDst) foeVertexDescriptorCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO,
        foeDestroyVertexDescriptorCreateInfo, sizeof(foeVertexDescriptorCreateInfo), &vdCI, dataFn,
        &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeVertexDescriptorCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_vertex_descriptor(foeVertexDescriptorCreateInfo const &vertexDescriptor)
    -> YAML::Node {
    YAML::Node definition;

    yaml_write_vertex_descriptor_internal("", vertexDescriptor, definition);

    return definition;
}