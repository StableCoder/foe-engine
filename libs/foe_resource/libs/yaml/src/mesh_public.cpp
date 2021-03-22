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

#include <foe/resource/yaml/mesh.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

bool import_yaml_mesh_definition(std::string_view name,
                                 std::string &fileName,
                                 std::string &meshName) {
    // Open the YAML file
    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(std::string{name} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
    }

    try {
        // Read the definition
        if (auto externalFileNode = rootNode["external_file"]; externalFileNode) {
            yaml_read_required("file", externalFileNode, fileName);
            yaml_read_required("mesh_name", externalFileNode, meshName);
        } /* else if (auto generatedCubeNode = rootNode["generated_cube"]; generatedCubeNode) {

         } else if (auto generatedIcosphereNode = rootNode["generated_icosphere"];
                    generatedIcosphereNode) {
         }*/
        else {
            return false;
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeMesh definition: {}", e.what());
        return false;
    }

    return true;
}