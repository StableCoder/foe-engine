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

#include <foe/resource/yaml/material.hpp>

#include <foe/ecs/id.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include "material.hpp"

#include <fstream>

bool import_yaml_material_definition(std::filesystem::path path,
                                     foeMaterialCreateInfo &createInfo) {
    // Open the YAML file
    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(path.native());
    } catch (YAML::ParserException const &e) {
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
        return false;
    } catch (YAML::BadFile const &e) {
        FOE_LOG(General, Fatal, "YAML::LoadFile failed: {}", e.what());
        return false;
    }

    try {
        return yaml_read_material_definition(
            "", rootNode, createInfo.fragmentShader, createInfo.fragDescriptorName,
            createInfo.image, createInfo.hasRasterizationSCI, createInfo.rasterizationSCI,
            createInfo.hasDepthStencilSCI, createInfo.depthStencilSCI, createInfo.hasColourBlendSCI,
            createInfo.colourBlendSCI, createInfo.colourBlendAttachments);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeFragmentDescriptor definition: {}", e.what());
        return false;
    }
}

bool export_yaml_material_definition(foeMaterial const *pMaterial) {
    YAML::Node definition;

    try {
        yaml_write_material_definition("", pMaterial, definition);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to export foeMaterial definition: {}", e.what());
        return false;
    }

    YAML::Emitter emitter;
    emitter << definition;

    std::ofstream outFile(std::string{"_"} + std::to_string(pMaterial->getID()) + ".yml",
                          std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
    } else {
        FOE_LOG(General, Error, "Failed to export foeMaterial: Failed to open output file {}.yml",
                foeId_to_string(pMaterial->getID()));
        return false;
    }

    return true;
}