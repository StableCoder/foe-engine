// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.hpp"

#include <foe/graphics/resource/mesh_create_info.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/model/assimp/flags.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

bool yaml_read_mesh_definition_internal(std::string const &nodeName,
                                        YAML::Node const &node,
                                        foeEcsGroupTranslator groupTranslator,
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
            *ci = {};

            yaml_read_required("file", externalFileNode, ci->fileName);
            yaml_read_required("mesh_name", externalFileNode, ci->meshName);

            // Post process flags
            std::string temp;
            yaml_read_optional("post_process", externalFileNode, temp);
            if (!foe_model_assimp_parse(temp, &ci->postProcessFlags)) {
                throw foeYamlException{
                    "external_file::post_process - Failed to parse post-processing flags"};
            }
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

            std::string serialized;
            if (!foe_model_assimp_serialize(pFileMesh->postProcessFlags, &serialized)) {
                throw foeYamlException{
                    "external_file::post_process - Failed to serialize post-processing flags"};
            }
            yaml_write_optional("post_process", std::string{}, serialized, fileNode);

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

char const *yaml_mesh_key() { return "mesh_v1"; }

void yaml_read_mesh(YAML::Node const &node,
                    foeEcsGroupTranslator groupTranslator,
                    foeResourceCreateInfo *pCreateInfo) {
    foeMeshCreateInfo meshCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_mesh_definition_internal(yaml_mesh_key(), node, groupTranslator, meshCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshCreateInfo *)pSrc;
        new (pDst) foeMeshCreateInfo(std::move(*pSrcData));
    };

    foeResult result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CREATE_INFO, foeDestroyMeshCreateInfo,
        sizeof(foeMeshCreateInfo), &meshCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{std::string{"Failed to create foeMeshCreateInfo due to error: "} +
                               buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_mesh(foeMeshCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_mesh_internal("", data, outNode);

    return outNode;
}