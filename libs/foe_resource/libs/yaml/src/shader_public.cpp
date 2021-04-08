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

#include <foe/ecs/id.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include "shader.hpp"

#include <fstream>

bool yaml_read_shader_definition(YAML::Node const &node, foeShaderCreateInfo &createInfo) {
    try {
        yaml_read_shader_definition("", node, createInfo.shaderCodeFile,
                                    createInfo.builtinSetLayouts, createInfo.descriptorSetLayoutCI,
                                    createInfo.setLayoutBindings, createInfo.pushConstantRange);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeShader definition: {}", e.what());
        return false;
    }

    return true;
}

void yaml_read_shader_definition2(YAML::Node const &node,
                                  foeResourceCreateInfoBase **ppCreateInfo) {
    foeShaderCreateInfo ci;

    yaml_read_shader_definition(node, ci);

    *ppCreateInfo = new foeShaderCreateInfo(std::move(ci));
}

bool export_yaml_shader_definition(foeGfxSession session, foeShader const *pShader) {
    YAML::Node definition;

    try {
        yaml_write_shader_definition("", session, pShader, definition);
    } catch (YAML::ParserException const &e) {
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
        return false;
    } catch (YAML::BadFile const &e) {
        FOE_LOG(General, Fatal, "YAML::LoadFile failed: {}", e.what());
        return false;
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{"_"} + std::to_string(pShader->getID()) + ".yml",
                          std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error, "Failed to export foeShader: Failed to open output file {}.yml",
                foeId_to_string(pShader->getID()));
        return false;
    }

    return true;
}