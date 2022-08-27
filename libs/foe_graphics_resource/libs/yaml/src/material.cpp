// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

char const *yaml_material_key() { return "material_v1"; }

void yaml_read_material(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo) {
    foeMaterialCreateInfo materialCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeMaterialCreateInfo(yaml_material_key(), node, groupTranslator, materialCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMaterialCreateInfo *)pSrc;
        new (pDst) foeMaterialCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)cleanup_foeMaterialCreateInfo,
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

    yaml_write_foeMaterialCreateInfo("", data, outNode);

    return outNode;
}