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

#include <foe/ecs/groups.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>

#include <fstream>

#include "entity.hpp"

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view indexDataFilePath = "index_data.yml";
constexpr std::string_view resourcesDirectoryPath = "resources";
constexpr std::string_view stateDataDirectoryPath = "state_data";

auto write_yaml_dependencies(foeEcsGroups &ecsGroups) -> YAML::Node {
    YAML::Node outNode;

    for (uint32_t i = 0; i < foeIdMaxDynamicGroups; ++i) {
        foeIdGroup groupID = foeIdValueToGroup(i);

        auto *pGroup = ecsGroups.group(groupID);
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

bool exportDependenciesToFile(std::filesystem::path path, foeEcsGroups &ecsGroups) {
    YAML::Node rootNode;

    try {
        rootNode = write_yaml_dependencies(ecsGroups);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to write general dependencies node with error: {}",
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
        rootNode = yaml_write_index_generator(data);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to write index data node with error: {}", e.what())
        return false;
    }

    YAML::Emitter emitter;
    emitter << rootNode;

    std::ofstream outFile{path, std::ofstream::out};
    outFile << emitter.c_str();

    return true;
}

bool exportGroupStateData(std::filesystem::path path,
                          foeEcsGroups &ecsGroups,
                          StatePools &statePools,
                          ResourcePools &resourcePools) {

    // Dependent groups
    for (uint32_t i = 0; i < foeIdMaxDynamicGroups; ++i) {
        foeIdGroup groupID = foeIdValueToGroup(i);
        auto *pGroup = ecsGroups.group(groupID);

        if (pGroup == nullptr)
            continue;

        auto groupEntityList = pGroup->activeEntityList();

        for (auto const entity : groupEntityList) {
            YAML::Node rootNode;

            try {
                rootNode = yaml_write_entity(entity, &statePools, &resourcePools);
            } catch (foeYamlException const &e) {
                FOE_LOG(General, Error,
                        "Failed to generete Yaml for entity {}: ", foeIdToString(entity), e.what())
                return false;
            }

            YAML::Emitter emitter;
            emitter << rootNode;

            std::ofstream outFile{path / std::string{foeIdToString(entity) + ".yml "},
                                  std::ofstream::out};
            outFile << emitter.c_str();
        }
    }

    { // Persistent group
        auto *pGroup = ecsGroups.persistentGroup();
        auto groupEntityList = pGroup->activeEntityList();

        for (auto const entity : groupEntityList) {
            YAML::Node rootNode;

            try {
                rootNode = yaml_write_entity(entity, &statePools, &resourcePools);
            } catch (foeYamlException const &e) {
                FOE_LOG(General, Error,
                        "Failed to generete Yaml for entity {}: ", foeIdToString(entity), e.what())
                return false;
            }

            YAML::Emitter emitter;
            emitter << rootNode;

            std::ofstream outFile{
                path / std::string{"P-" + foeIdToString(foeIdGetIndex(entity)) + ".yml "},
                std::ofstream::out};
            outFile << emitter.c_str();
        }
    }

    return true;
}

} // namespace

bool exportGroupState(std::filesystem::path yamlPath,
                      foeEcsGroups &ecsGroups,
                      StatePools &statePools,
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
        bool created = std::filesystem::create_directories(yamlPath / resourcesDirectoryPath, errC);
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

        created = std::filesystem::create_directories(yamlPath / stateDataDirectoryPath, errC);
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
    bool retVal = exportDependenciesToFile(yamlPath / dependenciesFilePath, ecsGroups);
    if (!retVal)
        return false;

    retVal = exportIndexDataToFile(yamlPath / indexDataFilePath, *ecsGroups.persistentGroup());
    if (!retVal)
        return false;

    retVal = exportGroupStateData(yamlPath / stateDataDirectoryPath, ecsGroups, statePools,
                                  resourcePools);
    if (!retVal)
        return false;

    return true;
}