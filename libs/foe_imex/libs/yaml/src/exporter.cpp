/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/imex/yaml/exporter.hpp>

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/error_code.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/indexes.hpp>
#include <foe/imex/importers.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "common.hpp"
#include "log.hpp"
#include "result.h"

#include <cstdint>
#include <fstream>
#include <shared_mutex>
#include <vector>

namespace {

std::shared_mutex gSync;

std::vector<std::vector<foeKeyYamlPair> (*)(foeResourceID, foeSimulation const *)> gResourceFns;
std::vector<std::vector<foeKeyYamlPair> (*)(foeEntityID, foeSimulation const *)> gComponentFns;

void emitYaml(std::filesystem::path emitPath, YAML::Node const &rootNode) {
    YAML::Emitter emitter;
    emitter << rootNode;

    std::ofstream outFile{emitPath, std::ofstream::out};
    outFile << emitter.c_str();
}

std::string id_to_filename(foeId id, std::string const &editorName) {
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
    if (!editorName.empty()) {
        filename += "_";
        filename += editorName;
    }

    return filename;
}

YAML::Node exportResource(
    foeResourceID resourceID,
    std::string const &name,
    std::vector<std::vector<foeKeyYamlPair> (*)(foeResourceID, foeSimulation const *)> const
        &resourceFns,
    foeSimulation const *pSimulation) {
    YAML::Node rootNode;

    yaml_write_id("", resourceID, rootNode);

    if (!name.empty()) {
        yaml_write_required("editor_name", name, rootNode);
    }

    for (auto const &fn : resourceFns) {
        auto keyDataPairs = fn(resourceID, pSimulation);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    return rootNode;
}

YAML::Node exportComponents(
    foeEntityID entity,
    std::string const &name,
    std::vector<std::vector<foeKeyYamlPair> (*)(foeEntityID, foeSimulation const *)> const
        &componentFns,
    foeSimulation const *pSimulation) {
    YAML::Node rootNode;

    yaml_write_id("", entity, rootNode);

    if (!name.empty()) {
        yaml_write_required("editor_name", name, rootNode);
    }

    for (auto const &fn : componentFns) {
        auto keyDataPairs = fn(entity, pSimulation);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    return rootNode;
}

foeResult exportDependencies(foeSimulation *pSimState, YAML::Node &data) {
    try {

        for (uint32_t i = 0; i < foeIdNumDynamicGroups; ++i) {
            foeIdGroup groupID = foeIdValueToGroup(i);

            foeImporterBase const *pGroup = pSimState->groupData.importer(groupID);
            if (pGroup == nullptr) {
                continue;
            }

            // Write the group info to the output node
            YAML::Node node;

            node["name"] = std::string{pGroup->name()};
            node["group_id"] = i;

            data.push_back(node);
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to write general dependencies node with exception: {}",
                e.what())
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES);
    }

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

bool exportIndexDataToFile(foeEcsIndexes indexes, YAML::Node &data) {
    try {
        yaml_write_indexes("", indexes, data);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to write index data node with exception: {}", e.what())
        return false;
    }

    return true;
}

foeResult exportGroupResourceIndexData(foeSimulation *pSimState, YAML::Node &data) {
    if (exportIndexDataToFile(pSimState->groupData.persistentResourceIndexes(), data))
        return to_foeResult(FOE_IMEX_YAML_SUCCESS);

    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA);
}

foeResult exportGroupEntityIndexData(foeSimulation *pSimState, YAML::Node &data) {
    if (exportIndexDataToFile(pSimState->groupData.persistentEntityIndexes(), data))
        return to_foeResult(FOE_IMEX_YAML_SUCCESS);

    return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA);
}

foeResult exportResources(foeIdGroup group, foeSimulation *pSimState, YAML::Node &data) {
    // Get the valid set of resource indices
    foeResult result;
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;

    do {
        uint32_t count;
        foeEcsExportIndexes(pSimState->groupData.resourceIndexes(group), nullptr, &count, nullptr);

        unusedIndices.resize(count);
        result = foeEcsExportIndexes(pSimState->groupData.resourceIndexes(group), &maxIndices,
                                     &count, unusedIndices.data());
        unusedIndices.resize(count);
    } while (result.value != FOE_SUCCESS);

    foeResourceID resourceID;
    auto unused = unusedIndices.begin();

    try {
        for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
            // Check if unused, then skip if it is
            if (unused != unusedIndices.end()) {
                if (idx == *unused)
                    continue;
            }

            resourceID = foeIdCreate(group, idx);

            // Resource Name
            char *pResourceName = NULL;
            if (pSimState->pResourceNameMap != nullptr) {
                uint32_t strLength = 0;
                foeResult result;
                do {
                    result =
                        pSimState->pResourceNameMap->find(resourceID, &strLength, pResourceName);
                    if (result.value == FOE_ECS_SUCCESS && pResourceName != NULL) {
                        break;
                    } else if ((result.value == FOE_ECS_SUCCESS && pResourceName == NULL) ||
                               result.value == FOE_ECS_INCOMPLETE) {
                        pResourceName = (char *)realloc(pResourceName, strLength);
                        if (pResourceName == NULL)
                            std::abort();
                    }
                } while (result.value != FOE_ECS_NO_MATCH);
            }

            data.push_back(exportResource(resourceID, pResourceName, gResourceFns, pSimState));
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export resource: {} - {}", foeIdToString(resourceID),
                e.what());
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA);
    }

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResult exportComponentData(foeIdGroup group, foeSimulation *pSimState, YAML::Node &data) {
    // Get the valid set of entity indices
    foeResult result;
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;

    do {
        uint32_t count;
        foeEcsExportIndexes(pSimState->groupData.entityIndexes(group), nullptr, &count, nullptr);

        unusedIndices.resize(count);
        result = foeEcsExportIndexes(pSimState->groupData.entityIndexes(group), &maxIndices, &count,
                                     unusedIndices.data());
        unusedIndices.resize(count);
    } while (result.value != FOE_SUCCESS);

    foeEntityID entity;
    auto unused = unusedIndices.begin();

    try {
        for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
            // Check if unused, then skip if it is
            if (unused != unusedIndices.end()) {
                if (idx == *unused)
                    continue;
            }

            entity = foeIdCreate(group, idx);

            // Entity Name
            char *pName = NULL;
            if (pSimState->pEntityNameMap != nullptr) {
                uint32_t strLength = 0;
                foeResult result;
                do {
                    result = pSimState->pEntityNameMap->find(entity, &strLength, pName);
                    if (result.value == FOE_ECS_SUCCESS && pName != NULL) {
                        break;
                    } else if ((result.value == FOE_ECS_SUCCESS && pName == NULL) ||
                               result.value == FOE_ECS_INCOMPLETE) {
                        pName = (char *)realloc(pName, strLength);
                        if (pName == NULL)
                            std::abort();
                    }
                } while (result.value != FOE_ECS_NO_MATCH);
            }

            data.push_back(exportComponents(entity, pName, gComponentFns, pSimState));
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export entity: {} - {}", foeIdToString(entity),
                e.what());
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA);
    }

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

} // namespace

foeResult foeImexYamlExport(std::filesystem::path rootPath, foeSimulation *pSimState) {
    // Make sure the export directory exists

    // Check if it exists already
    if (std::filesystem::exists(rootPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_directory(rootPath)) {
            FOE_LOG(foeImexYaml, Info,
                    "Attempting to export state via Yaml to an existing  directory at '{}'",
                    rootPath.string())
            std::filesystem::remove_all(rootPath);
        } else {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data to a non-directory location '{}', unsupported",
                    rootPath.string())
            return to_foeResult(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY);
        }
    }

    // Create the resource sub-directory
    std::error_code errC;

    bool created = std::filesystem::create_directories(rootPath, errC);
    if (errC) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, with error: {}",
                rootPath.string(), errC.message())
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                rootPath.string())
        return to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
    } else {
        FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                rootPath.string())
    }

    gSync.lock_shared();
    foeResult result;

    { // Dependency Data
        YAML::Node dependencies;
        result = exportDependencies(pSimState, dependencies);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
        emitYaml(rootPath / dependenciesFilePath, dependencies);
    }

    { // Resource Indices
        YAML::Node resourceIndices;
        result = exportGroupResourceIndexData(pSimState, resourceIndices);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
        emitYaml(rootPath / resourceIndexDataFilePath, resourceIndices);
    }

    { // Entity Indices
        YAML::Node entityIndices;
        result = exportGroupEntityIndexData(pSimState, entityIndices);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
        emitYaml(rootPath / entityIndexDataFilePath, entityIndices);
    }

    { // Resource Data
        YAML::Node resourceData;
        result = exportResources(0, pSimState, resourceData);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;

        // Make sure the export directory exists
        auto const dirPath = rootPath / resourceDirectoryPath;

        // Check if it exists already
        if (std::filesystem::exists(dirPath)) {
            // Determine if it's a file or a directory
            if (std::filesystem::is_regular_file(dirPath)) {
                FOE_LOG(foeImexYaml, Error,
                        "Attempted to export state data as a single Yaml file '{}', unsupported",
                        dirPath.string())
                result = to_foeResult(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY);
                goto EXPORT_FAILED;
            } else if (std::filesystem::is_directory(dirPath)) {
                FOE_LOG(foeImexYaml, Info,
                        "Attempting to export state via Yaml to an existing  directory at '{}'",
                        dirPath.string())
                std::filesystem::remove_all(dirPath);
            }
        }

        // Create the resource sub-directory
        bool created = std::filesystem::create_directories(dirPath, errC);
        if (errC) {
            FOE_LOG(foeImexYaml, Error,
                    "Failed to create directory '{}' to export Yaml state, with error: {}",
                    dirPath.string(), errC.message())
            result = to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
            goto EXPORT_FAILED;
        } else if (!created) {
            FOE_LOG(foeImexYaml, Error,
                    "Failed to create directory '{}' to export Yaml state, no error given",
                    dirPath.string())
            result = to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
            goto EXPORT_FAILED;
        } else {
            FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                    dirPath.string())
        }

        // Loop through the yaml list, emitting data
        for (auto const &it : resourceData) {
            std::string name;
            yaml_read_optional("editor_name", it, name);

            foeResourceID resourceID;
            yaml_read_id_required("", it, nullptr, resourceID);

            emitYaml(dirPath / std::string{id_to_filename(resourceID, name) + ".yml"}, it);
        }
    }

    { // Entity Data
        YAML::Node entityData;
        result = exportComponentData(0, pSimState, entityData);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;

        // Make sure the export directory exists
        auto const dirPath = rootPath / entityDirectoryPath;
        // Check if it exists already
        if (std::filesystem::exists(dirPath)) {
            // Determine if it's a file or a directory
            if (std::filesystem::is_regular_file(dirPath)) {
                FOE_LOG(foeImexYaml, Error,
                        "Attempted to export state data as a single Yaml file '{}', unsupported",
                        dirPath.string())
                result = to_foeResult(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY);
                goto EXPORT_FAILED;
            } else if (std::filesystem::is_directory(dirPath)) {
                FOE_LOG(foeImexYaml, Info,
                        "Attempting to export state via Yaml to an existing  directory at '{}'",
                        dirPath.string())
                std::filesystem::remove_all(dirPath);
            }
        }

        // Create the resource sub-directory
        bool created = std::filesystem::create_directories(dirPath, errC);
        if (errC) {
            FOE_LOG(foeImexYaml, Error,
                    "Failed to create directory '{}' to export Yaml state, with error: {}",
                    dirPath.string(), errC.message())
            result = to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
            goto EXPORT_FAILED;
        } else if (!created) {
            FOE_LOG(foeImexYaml, Error,
                    "Failed to create directory '{}' to export Yaml state, no error given",
                    dirPath.string())
            result = to_foeResult(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION);
            goto EXPORT_FAILED;
        } else {
            FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                    dirPath.string())
        }

        // Loop through the yaml list, emitting data
        for (auto const &it : entityData) {
            std::string name;
            yaml_read_optional("editor_name", it, name);

            foeEntityID entityID;
            yaml_read_id_required("", it, nullptr, entityID);

            emitYaml(dirPath / std::string{id_to_filename(entityID, name) + ".yml"}, it);
        }
    }

EXPORT_FAILED:
    gSync.unlock_shared();

    return result;
}

foeResult foeImexYamlRegisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *)) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gResourceFns) {
        if (it == pResourceFn) {
            return to_foeResult(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);
        }
    }

    gResourceFns.emplace_back(pResourceFn);
    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResult foeImexYamlDeregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *)) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gResourceFns.begin(), gResourceFns.end(), pResourceFn);
    if (searchIt != gResourceFns.end()) {
        gResourceFns.erase(searchIt);
        return to_foeResult(FOE_IMEX_YAML_SUCCESS);
    }

    return to_foeResult(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED);
}

foeResult foeImexYamlRegisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *)) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gComponentFns) {
        if (it == pComponentFn) {
            return to_foeResult(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);
        }
    }

    gComponentFns.emplace_back(pComponentFn);
    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

foeResult foeImexYamlDeregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *)) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gComponentFns.begin(), gComponentFns.end(), pComponentFn);
    if (searchIt != gComponentFns.end()) {
        gComponentFns.erase(searchIt);
        return to_foeResult(FOE_IMEX_YAML_SUCCESS);
    }

    return to_foeResult(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED);
}
