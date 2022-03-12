/*
    Copyright (C) 2021-2022 George Cave.

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

#include "image.hpp"

#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <memory>

namespace {

bool yaml_read_image_definition_internal(std::string const &nodeName,
                                         YAML::Node const &node,
                                         foeIdGroupTranslator const * /*pTranslator*/,
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
                     foeIdGroupTranslator const *pTranslator,
                     foeResourceCreateInfo *pCreateInfo) {
    foeImageCreateInfo imageCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_image_definition_internal(yaml_image_key(), node, pTranslator, imageCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeImageCreateInfo *)pSrc;
        new (pDst) foeImageCreateInfo(std::move(*pSrcData));
    };

    std::error_code errC = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO, foeDestroyImageCreateInfo,
        sizeof(foeImageCreateInfo), &imageCI, dataFn, &createInfo);
    if (errC) {
        throw foeYamlException{
            std::string{"Failed to create foeResourceCreateInfo due to error: "} + errC.message()};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_image(foeImageCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_image_internal("", data, outNode);

    return outNode;
}