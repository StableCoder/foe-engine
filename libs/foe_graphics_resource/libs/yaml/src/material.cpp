// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/resource/material_create_info.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

bool yaml_read_material_definition_internal(std::string const &nodeName,
                                            YAML::Node const &node,
                                            foeEcsGroupTranslator groupTranslator,
                                            foeMaterialCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read{false};
    try {
        // Resources
        read |= yaml_read_id_optional("fragment_shader", subNode, groupTranslator,
                                      createInfo.fragmentShader);
        read |= yaml_read_id_optional("image", subNode, groupTranslator, createInfo.image);

        VkPipelineRasterizationStateCreateInfo rasterizationSCI{};
        if (yaml_read_optional("rasterization", subNode, rasterizationSCI)) {
            createInfo.pRasterizationSCI =
                new VkPipelineRasterizationStateCreateInfo{rasterizationSCI};
        }

        VkPipelineDepthStencilStateCreateInfo depthStencilSCI{};
        if (yaml_read_optional("depth_stencil", subNode, depthStencilSCI)) {
            createInfo.pDepthStencilSCI =
                new VkPipelineDepthStencilStateCreateInfo{depthStencilSCI};
        }

        VkPipelineColorBlendStateCreateInfo colourBlendSCI{};
        if (yaml_read_optional("colour_blend", subNode, colourBlendSCI)) {
            createInfo.pColourBlendSCI = new VkPipelineColorBlendStateCreateInfo{colourBlendSCI};
        }

        read |= createInfo.pRasterizationSCI != nullptr || createInfo.pDepthStencilSCI != nullptr ||
                createInfo.pColourBlendSCI != nullptr;
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException(nodeName + "::" + e.what());
        }
    }

    return read;
}

void yaml_write_material_internal(std::string const &nodeName,
                                  foeMaterialCreateInfo const &data,
                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Fragment Shader
        if (data.fragmentShader != FOE_INVALID_ID) {
            yaml_write_id("fragment_shader", data.fragmentShader, writeNode);
        }

        // Image
        if (data.image != FOE_INVALID_ID) {
            yaml_write_id("image", data.image, writeNode);
        }

        // Fragment Descriptor Items
        if (data.pRasterizationSCI != nullptr)
            yaml_write_required("rasterization", *data.pRasterizationSCI, writeNode);

        if (data.pDepthStencilSCI != nullptr)
            yaml_write_required("depth_stencil", *data.pDepthStencilSCI, writeNode);

        if (data.pColourBlendSCI != nullptr)
            yaml_write_required("colour_blend", *data.pColourBlendSCI, writeNode);

    } catch (foeYamlException const &e) {
        if (nodeName.empty())
            throw e;
        else
            throw foeYamlException{nodeName + "::" + e.whatStr()};
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

} // namespace

char const *yaml_material_key() { return "material_v1"; }

void yaml_read_material(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo) {
    foeMaterialCreateInfo materialCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_material_definition_internal(yaml_material_key(), node, groupTranslator, materialCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMaterialCreateInfo *)pSrc;
        new (pDst) foeMaterialCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO, foeDestroyMaterialCreateInfo,
        sizeof(foeMaterialCreateInfo), &materialCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeMaterialCreateInfo due to error: "} + buffer};
    }

    materialCI = {};
    *pCreateInfo = createInfo;
}

auto yaml_write_material(foeMaterialCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_material_internal("", data, outNode);

    return outNode;
}