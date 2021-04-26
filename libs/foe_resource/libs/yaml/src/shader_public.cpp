/*
    Copyright (C) 2021 George Cave.

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

#include <foe/resource/yaml/shader.hpp>

#include <foe/graphics/yaml/shader.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "shader.hpp"

#include <fstream>

namespace {

bool yaml_read_shader_definition_internal(std::string const &nodeName,
                                          YAML::Node const &node,
                                          foeIdGroupTranslator const *pTranslator,
                                          foeShaderCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Resource Node
        // (Nothing currently)

        { // SPIR-V Source
            yaml_read_required("spirv_source", subNode, createInfo.shaderCodeFile);
        }

        // Gfx Data Node
        yaml_read_gfx_shader("graphics_data", subNode, createInfo.builtinSetLayouts,
                             createInfo.descriptorSetLayoutCI, createInfo.pushConstantRange);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

} // namespace

void yaml_read_shader_definition(YAML::Node const &node,
                                 foeIdGroupTranslator const *pTranslator,
                                 foeResourceCreateInfoBase **ppCreateInfo) {
    foeShaderCreateInfo ci;

    yaml_read_shader_definition_internal("", node, pTranslator, ci);

    *ppCreateInfo = new foeShaderCreateInfo(std::move(ci));
}

auto yaml_write_shader_definition(foeGfxSession session, foeShader const *pShader) -> YAML::Node {
    YAML::Node definition;

    yaml_write_shader_definition("", session, pShader, definition);

    return definition;
}