// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader.hpp"

#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/yaml/shader.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <string.h>

namespace {

bool yaml_read_shader_internal(std::string const &nodeName,
                               YAML::Node const &node,
                               foeEcsGroupTranslator groupTranslator,
                               foeShaderCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        std::string tempStr;
        yaml_read_required("spirv_source", subNode, tempStr);
        createInfo.pFile = (char *)malloc(tempStr.size() + 1);
        memcpy((char *)createInfo.pFile, tempStr.c_str(), tempStr.size() + 1);

        yaml_read_gfx_shader("graphics_data", subNode, createInfo.gfxCreateInfo);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

bool yaml_write_shader_internal(std::string const &nodeName,
                                foeShaderCreateInfo const &data,
                                YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required("spirv_source", std::string{data.pFile}, writeNode);

        yaml_write_gfx_shader("graphics_data", data.gfxCreateInfo, writeNode);
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeFragmentDescriptor' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}

} // namespace

char const *yaml_shader_key() { return "shader_v1"; }

void yaml_read_shader(YAML::Node const &node,
                      foeEcsGroupTranslator groupTranslator,
                      foeResourceCreateInfo *pCreateInfo) {
    foeShaderCreateInfo shaderCI;
    foeResourceCreateInfo createInfo;

    yaml_read_shader_internal(yaml_shader_key(), node, groupTranslator, shaderCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeShaderCreateInfo *)pSrc;
        new (pDst) foeShaderCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)foeCleanup_foeShaderCreateInfo,
        sizeof(foeShaderCreateInfo), &shaderCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{std::string{"Failed to create foeShaderCreateInfo due to error: "} +
                               buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_shader(foeShaderCreateInfo const &data) -> YAML::Node {
    YAML::Node definition;

    yaml_write_shader_internal("", data, definition);

    return definition;
}