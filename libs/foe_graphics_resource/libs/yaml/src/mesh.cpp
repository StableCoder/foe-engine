// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

namespace {

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

} // namespace

char const *yaml_mesh_file_key() { return "mesh_file_v1"; }

void yaml_read_mesh_file(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeResourceCreateInfo *pCreateInfo) {
    foeMeshFileCreateInfo meshCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeMeshFileCreateInfo(yaml_mesh_file_key(), node, meshCI);

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

    yaml_write_foeMeshFileCreateInfo("", data, outNode);

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

    yaml_read_foeMeshIcosphereCreateInfo(yaml_mesh_icosphere_key(), node, meshCI);

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

    yaml_write_foeMeshIcosphereCreateInfo("", data, outNode);

    return outNode;
}