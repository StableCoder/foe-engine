// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <foe/graphics/resource/image_create_info.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <memory>

namespace {

bool yaml_read_image_definition_internal(std::string const &nodeName,
                                         YAML::Node const &node,
                                         foeEcsGroupTranslator groupTranslator,
                                         foeImageCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        yaml_read_required("file", subNode, createInfo.fileName);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException(nodeName + "::" + e.what());
        }
    }

    return true;
}

void yaml_write_image_internal(std::string const &nodeName,
                               foeImageCreateInfo const &data,
                               YAML::Node &node) {

    YAML::Node writeNode;

    try {
        yaml_write_required("file", data.fileName, writeNode);
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

char const *yaml_image_key() { return "image_v1"; }

void yaml_read_image(YAML::Node const &node,
                     foeEcsGroupTranslator groupTranslator,
                     foeResourceCreateInfo *pCreateInfo) {
    foeImageCreateInfo imageCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_image_definition_internal(yaml_image_key(), node, groupTranslator, imageCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeImageCreateInfo *)pSrc;
        new (pDst) foeImageCreateInfo(std::move(*pSrcData));
    };

    foeResult result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO, foeDestroyImageCreateInfo,
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

    yaml_write_image_internal("", data, outNode);

    return outNode;
}