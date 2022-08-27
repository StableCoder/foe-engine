// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape.hpp"

#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <foe/physics/type_defs.h>
#include <foe/physics/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

char const *yaml_collision_shape_key() { return "collision_shape_v1"; }

void yaml_read_collision_shape(YAML::Node const &node,
                               foeEcsGroupTranslator groupTranslator,
                               foeResourceCreateInfo *pCreateInfo) {
    foeCollisionShapeCreateInfo ci{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeCollisionShapeCreateInfo(yaml_collision_shape_key(), node, ci);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeCollisionShapeCreateInfo *)pSrc;
        new (pDst) foeCollisionShapeCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result =
        foeCreateResourceCreateInfo(FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO, nullptr,
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

    yaml_write_foeCollisionShapeCreateInfo("", data, outNode);

    return outNode;
}