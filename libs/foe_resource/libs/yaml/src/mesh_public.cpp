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

bool import_yaml_mesh_definition(std::filesystem::path path, foeMeshCreateInfo &createInfo) {
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
        // Read the definition
        if (auto externalFileNode = rootNode["external_file"]; externalFileNode) {
            createInfo.source.reset(new foeMeshFileSource);
            foeMeshFileSource *ci = static_cast<foeMeshFileSource *>(createInfo.source.get());

            yaml_read_required("file", externalFileNode, ci->fileName);
            yaml_read_required("mesh_name", externalFileNode, ci->meshName);
        } else if (auto generatedCubeNode = rootNode["generated_cube"]; generatedCubeNode) {
            createInfo.source.reset(new foeMeshCubeSource);
            foeMeshCubeSource *ci = static_cast<foeMeshCubeSource *>(createInfo.source.get());

        } else if (auto generatedIcosphereNode = rootNode["generated_icosphere"];
                   generatedIcosphereNode) {
            createInfo.source.reset(new foeMeshIcosphereSource);
            foeMeshIcosphereSource *ci =
                static_cast<foeMeshIcosphereSource *>(createInfo.source.get());

            yaml_read_required("recursion", generatedIcosphereNode, ci->recursion);
        } else {
            return false;
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeMesh definition: {}", e.what());
        return false;
    }

    return true;
}