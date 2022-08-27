// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/model/assimp/flags.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <string.h>

namespace {

bool yaml_read_mesh_file_definition_internal(std::string const &nodeName,
                                             YAML::Node const &node,
                                             foeEcsGroupTranslator groupTranslator,
                                             foeMeshFileCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Read the definition
        createInfo = {};
        std::string tempStr;

        yaml_read_required("file", subNode, tempStr);
        createInfo.pFile = (char *)malloc(tempStr.size() + 1);
        memcpy((char *)createInfo.pFile, tempStr.c_str(), tempStr.size() + 1);

        yaml_read_required("mesh_name", subNode, tempStr);
        createInfo.pMesh = (char *)malloc(tempStr.size() + 1);
        memcpy((char *)createInfo.pMesh, tempStr.c_str(), tempStr.size() + 1);

        // Post process flags
        std::string temp;
        yaml_read_optional("post_process", subNode, temp);
        if (!foe_model_assimp_parse(temp, &createInfo.postProcessFlags)) {
            throw foeYamlException{"post_process - Failed to parse post-processing flags"};
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

bool yaml_read_mesh_cube_definition_internal(std::string const &nodeName,
                                             YAML::Node const &node,
                                             foeEcsGroupTranslator groupTranslator,
                                             foeMeshCubeCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Read the definition
        createInfo = {};
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    return true;
}

bool yaml_read_mesh_icosphere_definition_internal(std::string const &nodeName,
                                                  YAML::Node const &node,
                                                  foeEcsGroupTranslator groupTranslator,
                                                  foeMeshIcosphereCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Read the definition
        createInfo = {};

        yaml_read_required("recursion", subNode, createInfo.recursion);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    return true;
}

void yaml_write_mesh_file_internal(std::string const &nodeName,
                                   foeMeshFileCreateInfo const &data,
                                   YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required("file", std::string{data.pFile}, writeNode);
        yaml_write_required("mesh_name", std::string{data.pMesh}, writeNode);

        std::string serialized;
        if (!foe_model_assimp_serialize(data.postProcessFlags, &serialized)) {
            throw foeYamlException{"post_process - Failed to serialize post-processing flags"};
        }
        yaml_write_optional("post_process", std::string{}, serialized, writeNode);
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

void yaml_write_mesh_cube_internal(std::string const &nodeName,
                                   foeMeshCubeCreateInfo const &data,
                                   YAML::Node &node) {
    YAML::Node writeNode;

    try {
        writeNode = YAML::Node{};
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

void yaml_write_mesh_icosphere_internal(std::string const &nodeName,
                                        foeMeshIcosphereCreateInfo const &data,
                                        YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required("recusrion", data.recursion, writeNode);
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

char const *yaml_mesh_file_key() { return "mesh_file_v1"; }

void yaml_read_mesh_file(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeResourceCreateInfo *pCreateInfo) {
    foeMeshFileCreateInfo meshCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_mesh_file_definition_internal(yaml_mesh_file_key(), node, groupTranslator, meshCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshFileCreateInfo *)pSrc;
        new (pDst) foeMeshFileCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeMeshFileCreateInfo,
                                    sizeof(foeMeshFileCreateInfo), &meshCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{std::string{"Failed to create foeMeshCreateInfo due to error: "} +
                               buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_mesh_file(foeMeshFileCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_mesh_file_internal("", data, outNode);

    return outNode;
}

char const *yaml_mesh_cube_key() { return "mesh_cube_v1"; }

void yaml_read_mesh_cube(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeResourceCreateInfo *pCreateInfo) {
    foeMeshCubeCreateInfo meshCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_mesh_cube_definition_internal(yaml_mesh_cube_key(), node, groupTranslator, meshCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshCubeCreateInfo *)pSrc;
        new (pDst) foeMeshCubeCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO, nullptr,
        sizeof(foeMeshCubeCreateInfo), &meshCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeMeshCubeCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_mesh_cube(foeMeshCubeCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_mesh_cube_internal("", data, outNode);

    return outNode;
}

char const *yaml_mesh_icosphere_key() { return "mesh_icosphere_v1"; }

void yaml_read_mesh_icosphere(YAML::Node const &node,
                              foeEcsGroupTranslator groupTranslator,
                              foeResourceCreateInfo *pCreateInfo) {
    foeMeshIcosphereCreateInfo meshCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_mesh_icosphere_definition_internal(yaml_mesh_icosphere_key(), node, groupTranslator,
                                                 meshCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshIcosphereCreateInfo *)pSrc;
        new (pDst) foeMeshIcosphereCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO, nullptr,
        sizeof(foeMeshIcosphereCreateInfo), &meshCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeMeshIcosphereCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_mesh_icosphere(foeMeshIcosphereCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_mesh_icosphere_internal("", data, outNode);

    return outNode;
}