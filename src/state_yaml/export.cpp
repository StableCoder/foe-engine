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

#include "export.hpp"

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/resource/yaml/armature.hpp>
#include <foe/resource/yaml/image.hpp>
#include <foe/resource/yaml/material.hpp>
#include <foe/resource/yaml/mesh.hpp>
#include <foe/resource/yaml/shader.hpp>
#include <foe/resource/yaml/vertex_descriptor.hpp>
#include <foe/simulation/group_data.hpp>
#include <foe/yaml/exception.hpp>

#include "../log.hpp"
#include "entity.hpp"

#include <fstream>
#include <sstream>

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view resourceIndexDataFilePath = "resource_index_data.yml";
constexpr std::string_view resourceDirectoryPath = "resources";
constexpr std::string_view stateIndexDataFilePath = "state_index_data.yml";
constexpr std::string_view stateDirectoryPath = "state";

std::string id_to_filename(foeId id, foeEditorNameMap *pNameMap) {
    std::string filename;

    { // Group
        foeIdGroupValue groupValue = foeIdGroupToValue(id);
        if (groupValue != foeIdPersistentGroupValue) {
            constexpr int groupWidth = (foeIdNumGroupBits / 4) + ((foeIdNumGroupBits % 4) ? 1 : 0);

            std::stringstream ss;
            ss << std::hex << std::uppercase << std::setfill('0');
            ss << "0x" << std::setw(groupWidth) << groupValue;
            filename += ss.str();
        }
    }

    filename += "_";

    // Index
    {
        constexpr int indexWidth = (foeIdNumIndexBits / 4) + ((foeIdNumIndexBits % 4) ? 1 : 0);
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        ss << "0x" << std::setw(indexWidth) << foeIdIndexToValue(id);
        filename += ss.str();
    }

    // Editor Name
    if (pNameMap != nullptr) {
        auto name = pNameMap->find(id);

        if (!name.empty()) {
            filename += "_";
            filename += name;
        }
    }

    return filename;
}

auto write_yaml_dependencies(foeGroupData &groups) -> YAML::Node {
    YAML::Node outNode;

    for (uint32_t i = 0; i < foeIdNumDynamicGroups; ++i) {
        foeIdGroup groupID = foeIdValueToGroup(i);

        foeImporterBase const *pGroup = groups.importer(groupID);
        if (pGroup == nullptr) {
            continue;
        }

        // Write the group info to the output node
        YAML::Node node;

        node["name"] = std::string{pGroup->name()};
        node["group_id"] = i;

        outNode.push_back(node);
    }

    return outNode;
}

bool exportDependenciesToFile(std::filesystem::path path, foeGroupData &groups) {
    YAML::Node rootNode;

    try {
        rootNode = write_yaml_dependencies(groups);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to write general dependencies node with exception: {}",
                e.what())
        return false;
    }

    YAML::Emitter emitter;
    emitter << rootNode;

    std::ofstream outFile{path, std::ofstream::out};
    outFile << emitter.c_str();

    return true;
}

bool exportIndexDataToFile(std::filesystem::path path, foeIdIndexGenerator &data) {
    YAML::Node rootNode;

    try {
        yaml_write_index_generator("", data, rootNode);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to write index data node with exception: {}", e.what())
        return false;
    }

    YAML::Emitter emitter;
    emitter << rootNode;

    std::ofstream outFile{path, std::ofstream::out};
    outFile << emitter.c_str();

    return true;
}

bool exportResources(std::filesystem::path path,
                     foeGfxSession gfxSession,
                     foeGroupData &groups,
                     foeEditorNameMap *pNameMap,
                     ResourcePools &resourcePools) {
    { // Persistent group

        // ARMATURE
        for (auto const it : resourcePools.armature.getDataVector()) {
            YAML::Node rootNode;

            try {
                if (it->createInfo != nullptr) {
                    rootNode = yaml_write_armature_definition(*it->createInfo.get());
                } else {
                    return false;
                }
            } catch (...) {
                return false;
            }

            yaml_write_id("", it->getID(), rootNode);

            if (pNameMap != nullptr) {
                auto name = pNameMap->find(it->getID());
                if (!name.empty()) {
                    yaml_write_required("editor_name", name, rootNode);
                }
            }

            YAML::Emitter emitter;
            emitter << rootNode;

            std::ofstream outFile{path /
                                      std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                                  std::ofstream::out};
            outFile << emitter.c_str();
        }
    }

    // IMAGE
    for (auto const it : resourcePools.image.getDataVector()) {
        YAML::Node rootNode;

        try {
            if (it->createInfo != nullptr) {
                rootNode = yaml_write_image_definition(*it->createInfo.get());
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }

        yaml_write_id("", it->getID(), rootNode);

        if (pNameMap != nullptr) {
            auto name = pNameMap->find(it->getID());
            if (!name.empty()) {
                yaml_write_required("editor_name", name, rootNode);
            }
        }

        YAML::Emitter emitter;
        emitter << rootNode;

        std::ofstream outFile{path / std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                              std::ofstream::out};
        outFile << emitter.c_str();
    }

    // MATERIAL
    for (auto const it : resourcePools.material.getDataVector()) {
        YAML::Node rootNode;

        try {
            if (it->createInfo != nullptr) {
                rootNode = yaml_write_material_definition(*it->createInfo.get(),
                                                          it->getGfxFragmentDescriptor());
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }

        yaml_write_id("", it->getID(), rootNode);

        if (pNameMap != nullptr) {
            auto name = pNameMap->find(it->getID());
            if (!name.empty()) {
                yaml_write_required("editor_name", name, rootNode);
            }
        }

        YAML::Emitter emitter;
        emitter << rootNode;

        std::ofstream outFile{path / std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                              std::ofstream::out};
        outFile << emitter.c_str();
    }

    // MESH
    for (auto const it : resourcePools.mesh.getDataVector()) {
        YAML::Node rootNode;

        try {
            if (it->createInfo != nullptr) {
                rootNode = yaml_write_mesh_definition(*it->createInfo.get());
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }

        yaml_write_id("", it->getID(), rootNode);

        if (pNameMap != nullptr) {
            auto name = pNameMap->find(it->getID());
            if (!name.empty()) {
                yaml_write_required("editor_name", name, rootNode);
            }
        }

        YAML::Emitter emitter;
        emitter << rootNode;

        std::ofstream outFile{path / std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                              std::ofstream::out};
        outFile << emitter.c_str();
    }

    // SHADER
    for (auto const it : resourcePools.shader.getDataVector()) {
        YAML::Node rootNode;

        try {
            if (it->createInfo != nullptr) {
                rootNode = yaml_write_shader_definition(*it->createInfo);
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }

        yaml_write_id("", it->getID(), rootNode);

        if (pNameMap != nullptr) {
            auto name = pNameMap->find(it->getID());
            if (!name.empty()) {
                yaml_write_required("editor_name", name, rootNode);
            }
        }

        YAML::Emitter emitter;
        emitter << rootNode;

        std::ofstream outFile{path / std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                              std::ofstream::out};
        outFile << emitter.c_str();
    }

    // VERTEX DESCRIPTORS
    for (auto const it : resourcePools.vertexDescriptor.getDataVector()) {
        YAML::Node rootNode;

        try {
            if (it->createInfo != nullptr) {
                rootNode = yaml_write_vertex_descriptor_definition(*it);
            } else {
                return false;
            }
        } catch (...) {
            return false;
        }

        yaml_write_id("", it->getID(), rootNode);

        if (pNameMap != nullptr) {
            auto name = pNameMap->find(it->getID());
            if (!name.empty()) {
                yaml_write_required("editor_name", name, rootNode);
            }
        }

        YAML::Emitter emitter;
        emitter << rootNode;

        std::ofstream outFile{path / std::string{id_to_filename(it->getID(), pNameMap) + ".yml"},
                              std::ofstream::out};
        outFile << emitter.c_str();
    }

    return true;
}

bool exportGroupStateData(std::filesystem::path path,
                          foeGroupData &groups,
                          foeEditorNameMap *pNameMap,
                          StatePools &statePools,
                          ResourcePools &resourcePools) {

    // Dependent groups
    for (uint32_t i = 0; i < foeIdNumDynamicGroups; ++i) {
        foeIdGroup idGroup = foeIdValueToGroup(i);
        auto *pGroup = groups.entityIndices(idGroup);

        if (pGroup == nullptr)
            continue;

        foeIdIndex nextFreeIndex;
        std::vector<foeIdIndex> recycled;
        pGroup->exportState(nextFreeIndex, recycled);

        auto recycledIt = recycled.begin();
        for (foeIdIndex idIndex = foeIdIndexMinValue; idIndex < nextFreeIndex; ++idIndex) {
            // Skip recycled IDs
            if (recycledIt != recycled.end() && *recycledIt == idIndex) {
                ++recycledIt;
                continue;
            }
            foeId entity = foeIdCreate(idGroup, idIndex);
            YAML::Node rootNode;

            try {
                rootNode = yaml_write_entity(entity, pNameMap, &statePools);
            } catch (foeYamlException const &e) {
                FOE_LOG(General, Error,
                        "Failed to generete Yaml for entity {}: ", foeIdToString(entity), e.what())
                return false;
            }

            YAML::Emitter emitter;
            emitter << rootNode;

            std::ofstream outFile{path / std::string{id_to_filename(entity, pNameMap) + ".yml"},
                                  std::ofstream::out};
            outFile << emitter.c_str();
        }
    }

    { // Persistent group
        auto *pGroup = groups.persistentEntityIndices();

        foeIdIndex nextFreeIndex;
        std::vector<foeIdIndex> recycled;
        pGroup->exportState(nextFreeIndex, recycled);

        auto recycledIt = recycled.begin();
        for (foeIdIndex idIndex = foeIdIndexMinValue; idIndex < nextFreeIndex; ++idIndex) {
            // Skip recycled IDs
            if (recycledIt != recycled.end() && *recycledIt == idIndex) {
                ++recycledIt;
                continue;
            }
            foeId entity = foeIdCreate(foeIdPersistentGroup, idIndex);
            YAML::Node rootNode;

            try {
                rootNode = yaml_write_entity(entity, pNameMap, &statePools);
            } catch (foeYamlException const &e) {
                FOE_LOG(General, Error,
                        "Failed to generete Yaml for entity {}: ", foeIdToString(entity), e.what())
                return false;
            }

            YAML::Emitter emitter;
            emitter << rootNode;

            std::ofstream outFile{path / std::string{id_to_filename(entity, pNameMap) + ".yml"},
                                  std::ofstream::out};
            outFile << emitter.c_str();
        }
    }

    return true;
}

} // namespace

bool exportGroupState(std::filesystem::path yamlPath,
                      foeGfxSession gfxSession,
                      foeGroupData &groups,
                      foeEditorNameMap *pEntityNameMap,
                      StatePools &statePools,
                      foeEditorNameMap *pResourceNameMap,
                      ResourcePools &resourcePools) {
    // Check if it exists already
    if (std::filesystem::exists(yamlPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(yamlPath)) {
            FOE_LOG(General, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    yamlPath.string())
            return false;
        } else if (std::filesystem::is_directory(yamlPath)) {
            FOE_LOG(General, Info,
                    "Attempting to export state via Yaml to an existing  directory at '{}'",
                    yamlPath.string())
            std::filesystem::remove_all(yamlPath);
        }
    } else {
        // Create the starting directory
        std::error_code errC;

        bool created = std::filesystem::create_directories(yamlPath, errC);
        if (errC) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, with error: {}",
                    yamlPath.string(), errC.message())
            return false;
        } else if (!created) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, no error given",
                    yamlPath.string())
            return false;
        } else {
            FOE_LOG(General, Info, "Created new directory at '{}' to export state as Yaml",
                    yamlPath.string())
        }
    }

    { // Create the required sub-directories
        std::error_code errC;
        bool created = std::filesystem::create_directories(yamlPath / resourceDirectoryPath, errC);
        if (errC) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, with error: {}",
                    yamlPath.string(), errC.message())
            return false;
        } else if (!created) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, no error given",
                    yamlPath.string())
            return false;
        }

        created = std::filesystem::create_directories(yamlPath / stateDirectoryPath, errC);
        if (errC) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, with error: {}",
                    yamlPath.string(), errC.message())
            return false;
        } else if (!created) {
            FOE_LOG(General, Error,
                    "Failed to create directory '{}' to export Yaml state, no error given",
                    yamlPath.string())
            return false;
        }
    }

    // Dependencies
    bool retVal = exportDependenciesToFile(yamlPath / dependenciesFilePath, groups);
    if (!retVal)
        return false;

    // Resource Index Data
    retVal = exportIndexDataToFile(yamlPath / resourceIndexDataFilePath,
                                   *groups.persistentResourceIndices());
    if (!retVal)
        return false;

    // Resources
    retVal = exportResources(yamlPath / resourceDirectoryPath, gfxSession, groups, pResourceNameMap,
                             resourcePools);
    if (!retVal)
        return false;

    // Entity Index Data
    retVal =
        exportIndexDataToFile(yamlPath / stateIndexDataFilePath, *groups.persistentEntityIndices());
    if (!retVal)
        return false;

    // Entity Data
    retVal = exportGroupStateData(yamlPath / stateDirectoryPath, groups, pEntityNameMap, statePools,
                                  resourcePools);
    if (!retVal)
        return false;

    return true;
}