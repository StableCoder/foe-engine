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

#include "shader.hpp"

#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/yaml/shader.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

bool yaml_read_shader_internal(std::string const &nodeName,
                               YAML::Node const &node,
                               foeIdGroupTranslator const *pTranslator,
                               foeShaderCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        yaml_read_required("spirv_source", subNode, createInfo.shaderCodeFile);

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
        yaml_write_required("spirv_source", data.shaderCodeFile, writeNode);

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
                      foeIdGroupTranslator const *pTranslator,
                      foeResourceCreateInfo *pCreateInfo) {
    foeShaderCreateInfo shaderCI;
    foeResourceCreateInfo createInfo;

    yaml_read_shader_internal(yaml_shader_key(), node, pTranslator, shaderCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeShaderCreateInfo *)pSrc;
        new (pDst) foeShaderCreateInfo(std::move(*pSrcData));
    };

    std::error_code errC = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO, foeDestroyShaderCreateInfo,
        sizeof(foeShaderCreateInfo), &shaderCI, dataFn, &createInfo);
    if (errC) {
        throw foeYamlException{std::string{"Failed to create foeShaderCreateInfo due to error: "} +
                               errC.message()};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_shader(foeShaderCreateInfo const &data) -> YAML::Node {
    YAML::Node definition;

    yaml_write_shader_internal("", data, definition);

    return definition;
}