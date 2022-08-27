// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

char const *yaml_image_key() { return "image_v1"; }

void yaml_read_image(YAML::Node const &node,
                     foeEcsGroupTranslator groupTranslator,
                     foeResourceCreateInfo *pCreateInfo) {
    foeImageCreateInfo imageCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeImageCreateInfo(yaml_image_key(), node, imageCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeImageCreateInfo *)pSrc;
        new (pDst) foeImageCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeImageCreateInfo,
                                    sizeof(foeImageCreateInfo), &imageCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeResourceCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_image(foeImageCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_foeImageCreateInfo("", data, outNode);

    return outNode;
}