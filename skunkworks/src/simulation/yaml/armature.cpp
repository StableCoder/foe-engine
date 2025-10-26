// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature.hpp"

#include <foe/yaml/exception.hpp>

#include "../armature_create_info.h"
#include "../cleanup.h"
#include "../type_defs.h"
#include "structs.hpp"

#include <string.h>

char const *yaml_armature_key() { return "armature_v1"; }

void yaml_read_armature(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo) {
    foeArmatureCreateInfo armatureCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeArmatureCreateInfo(yaml_armature_key(), node, armatureCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeArmatureCreateInfo *)pSrc;
        new (pDst) foeArmatureCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)cleanup_foeArmatureCreateInfo,
        sizeof(foeArmatureCreateInfo), &armatureCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeArmatureCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_armature(foeArmatureCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_foeArmatureCreateInfo("", data, outNode);

    return outNode;
}