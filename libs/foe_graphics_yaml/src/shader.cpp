/*
    Copyright (C) 2020 George Cave.

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

#include <foe/graphics/yaml/shader.hpp>

#include <foe/graphics/shader.hpp>
#include <foe/graphics/shader_pool.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_read_shader(std::string const &nodeName,
                      YAML::Node const &node,
                      foeShaderPool *pShaderPool,
                      foeShader **pShader) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        std::string shaderName;
        yaml_read_required("file_shader", subNode, shaderName);

        *pShader = pShaderPool->create(shaderName);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

bool yaml_write_shader(std::string const &nodeName, foeShader const *pShader, YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node["file_shader"] = pShader->name;
        } else {
            node[nodeName]["file_shader"] = pShader->name;
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeShader'");
    }

    return true;
}