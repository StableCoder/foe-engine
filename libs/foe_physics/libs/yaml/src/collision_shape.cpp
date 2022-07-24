// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape.hpp"

#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <foe/physics/type_defs.h>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <string_view>

namespace {

bool yaml_read_collision_shape_definition_internal(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   foeEcsGroupTranslator groupTranslator,
                                                   foeCollisionShapeCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        yaml_read_required("box_size", subNode, createInfo.boxSize);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    return true;
}

void yaml_write_collision_shape_definition_internal(std::string const &nodeName,
                                                    foeCollisionShapeCreateInfo const &data,
                                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required("box_size", data.boxSize, writeNode);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

} // namespace

char const *yaml_collision_shape_key() { return "collision_shape_v1"; }

void yaml_read_collision_shape(YAML::Node const &node,
                               foeEcsGroupTranslator groupTranslator,
                               foeResourceCreateInfo *pCreateInfo) {
    foeCollisionShapeCreateInfo ci{};
    foeResourceCreateInfo createInfo;

    yaml_read_collision_shape_definition_internal(yaml_collision_shape_key(), node, groupTranslator,
                                                  ci);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeCollisionShapeCreateInfo *)pSrc;
        new (pDst) foeCollisionShapeCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO, foeDestroyCollisionShapeCreateInfo,
        sizeof(foeCollisionShapeCreateInfo), &ci, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeCollisionShapeCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_collision_shape(foeCollisionShapeCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_collision_shape_definition_internal("", data, outNode);

    return outNode;
}