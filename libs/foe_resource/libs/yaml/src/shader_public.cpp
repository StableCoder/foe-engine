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

#include <foe/resource/imex/shader.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include "shader.hpp"

#include <fstream>

bool import_shader_definition(std::string_view shaderName,
                              std::string &shaderCodeFile,
                              foeBuiltinDescriptorSetLayoutFlags &builtinSetLayouts,
                              VkDescriptorSetLayoutCreateInfo &descriptorSetLayoutCI,
                              VkPushConstantRange &pushConstantRange) {
    // Open the YAML file
    YAML::Node config;
    try {
        config = YAML::LoadFile(std::string{shaderName} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load config file: {}", e.what());
    }

    try {
        yaml_read_shader_definition("", config, shaderCodeFile, builtinSetLayouts,
                                    descriptorSetLayoutCI, pushConstantRange);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeShader definition: {}", e.what());
        return false;
    }

    return true;
}

bool export_shader_definition(foeGfxSession session, foeShader const *pShader) {
    YAML::Node definition;

    try {
        yaml_write_shader_definition("", session, pShader, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeShader definition: {}", e.what());
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{pShader->getName()} + ".yml", std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error, "Failed to export foeShader: Failed to open output file {}.yml",
                pShader->getName());
        return false;
    }

    return true;
}