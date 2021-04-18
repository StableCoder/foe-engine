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

bool yaml_read_mesh_definition(YAML::Node const &node,
                               foeIdGroupTranslator const *pTranslator,
                               foeMeshCreateInfo &createInfo) {
    try {
        // Read the definition
        if (auto externalFileNode = node["external_file"]; externalFileNode) {
            createInfo.source.reset(new foeMeshFileSource);
            foeMeshFileSource *ci = static_cast<foeMeshFileSource *>(createInfo.source.get());

            yaml_read_required("file", externalFileNode, ci->fileName);
            yaml_read_required("mesh_name", externalFileNode, ci->meshName);
        } else if (auto generatedCubeNode = node["generated_cube"]; generatedCubeNode) {
            createInfo.source.reset(new foeMeshCubeSource);
            foeMeshCubeSource *ci = static_cast<foeMeshCubeSource *>(createInfo.source.get());

        } else if (auto generatedIcosphereNode = node["generated_icosphere"];
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

void yaml_read_mesh_definition2(YAML::Node const &node,
                                foeIdGroupTranslator const *pTranslator,
                                foeResourceCreateInfoBase **ppCreateInfo) {
    foeMeshCreateInfo ci;

    yaml_read_mesh_definition(node, pTranslator, ci);

    *ppCreateInfo = new foeMeshCreateInfo(std::move(ci));
}