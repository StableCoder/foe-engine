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

#include "mesh.hpp"

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

constexpr std::string_view cNodeName = "mesh_v1";

bool yaml_read_mesh_definition_internal(std::string const &nodeName,
                                        YAML::Node const &node,
                                        foeIdGroupTranslator const *pTranslator,
                                        foeMeshCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Read the definition
        if (auto externalFileNode = subNode["external_file"]; externalFileNode) {
            createInfo.source.reset(new foeMeshFileSource);
            foeMeshFileSource *ci = static_cast<foeMeshFileSource *>(createInfo.source.get());

            yaml_read_required("file", externalFileNode, ci->fileName);
            yaml_read_required("mesh_name", externalFileNode, ci->meshName);
        } else if (auto generatedCubeNode = subNode["generated_cube"]; generatedCubeNode) {
            createInfo.source.reset(new foeMeshCubeSource);
            foeMeshCubeSource *ci = static_cast<foeMeshCubeSource *>(createInfo.source.get());
        } else if (auto generatedIcosphereNode = subNode["generated_icosphere"];
                   generatedIcosphereNode) {
            createInfo.source.reset(new foeMeshIcosphereSource);
            foeMeshIcosphereSource *ci =
                static_cast<foeMeshIcosphereSource *>(createInfo.source.get());

            yaml_read_required("recursion", generatedIcosphereNode, ci->recursion);
        } else {
            return false;
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    return true;
}

void yaml_write_mesh_internal(std::string const &nodeName,
                              foeMeshCreateInfo const &data,
                              YAML::Node &node) {
    YAML::Node writeNode;

    try {
        if (auto *pFileMesh = dynamic_cast<foeMeshFileSource *>(data.source.get()); pFileMesh) {
            YAML::Node fileNode;
            yaml_write_required("file", pFileMesh->fileName, fileNode);
            yaml_write_required("mesh_name", pFileMesh->meshName, fileNode);
            writeNode["external_file"] = fileNode;

        } else if (auto *pCube = dynamic_cast<foeMeshCubeSource *>(data.source.get()); pCube) {
            writeNode["generated_cube"] = YAML::Node{};

        } else if (auto *pSphere = dynamic_cast<foeMeshIcosphereSource *>(data.source.get());
                   pSphere) {
            YAML::Node icosphereNode;
            yaml_write_required("recusrion", pSphere->recursion, icosphereNode);
            writeNode["generated_icosphere"] = icosphereNode;
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

} // namespace

void yaml_read_mesh_definition(YAML::Node const &node,
                               foeIdGroupTranslator const *pTranslator,
                               foeResourceCreateInfoBase **ppCreateInfo) {
    foeMeshCreateInfo ci;

    yaml_read_mesh_definition_internal(std::string{cNodeName}, node, pTranslator, ci);

    *ppCreateInfo = new foeMeshCreateInfo(std::move(ci));
}

auto yaml_write_mesh_definition(foeMeshCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_mesh_internal("", data, outNode);

    return outNode;
}