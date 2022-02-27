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
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/imex/importers.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "common.hpp"
#include "error_code.hpp"
#include "log.hpp"

#include <cstdint>
#include <fstream>
#include <shared_mutex>
#include <vector>

namespace {

std::shared_mutex gSync;

std::vector<std::vector<foeKeyYamlPair> (*)(foeResourceID, foeSimulationState const *)>
    gResourceFns;
std::vector<std::vector<foeKeyYamlPair> (*)(foeEntityID, foeSimulationState const *)> gComponentFns;

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

void exportResource(
    foeResourceID resource,
    std::filesystem::path directory,
    foeEditorNameMap *pNameMap,
    std::vector<std::vector<foeKeyYamlPair> (*)(foeResourceID, foeSimulationState const *)> const
        &resourceFns,
    foeSimulationState const *pSimulationState) {
    std::string editorName;
    if (pNameMap != nullptr) {
        editorName = pNameMap->find(resource);
    }

    YAML::Node rootNode;

    yaml_write_id("", resource, rootNode);

    if (!editorName.empty()) {
        yaml_write_required("editor_name", editorName, rootNode);
    }

    for (auto const &fn : resourceFns) {
        auto keyDataPairs = fn(resource, pSimulationState);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    emitYaml(directory / std::string{id_to_filename(resource, editorName) + ".yml"}, rootNode);
}

void exportComponents(
    foeEntityID entity,
    std::filesystem::path directory,
    foeEditorNameMap *pNameMap,
    std::vector<std::vector<foeKeyYamlPair> (*)(foeEntityID, foeSimulationState const *)> const
        &componentFns,
    foeSimulationState const *pSimulationState) {
    std::string editorName;
    if (pNameMap != nullptr) {
        editorName = pNameMap->find(entity);
    }

    YAML::Node rootNode;

    yaml_write_id("", entity, rootNode);

    if (!editorName.empty()) {
        yaml_write_required("editor_name", editorName, rootNode);
    }

    for (auto const &fn : componentFns) {
        auto keyDataPairs = fn(entity, pSimulationState);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    emitYaml(directory / std::string{id_to_filename(entity, editorName) + ".yml"}, rootNode);
}

std::error_code exportDependencies(std::filesystem::path rootOutPath,
                                   foeSimulationState *pSimState) {
    try {
        YAML::Node rootNode;

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

            rootNode.push_back(node);
        }

        emitYaml(rootOutPath / dependenciesFilePath, rootNode);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to write general dependencies node with exception: {}",
                e.what())
        return FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES;
    }

    return FOE_IMEX_YAML_SUCCESS;
}

bool exportIndexDataToFile(std::filesystem::path path, foeIdIndexGenerator *pData) {
    YAML::Node rootNode;

    try {
        yaml_write_index_generator("", *pData, rootNode);
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to write index data node with exception: {}", e.what())
        return false;
    }

    emitYaml(path, rootNode);

    return true;
}

std::error_code exportGroupResourceIndexData(std::filesystem::path rootOutPath,
                                             foeSimulationState *pSimState) {
    if (exportIndexDataToFile(rootOutPath / resourceIndexDataFilePath,
                              pSimState->groupData.persistentResourceIndices()))
        return FOE_IMEX_YAML_SUCCESS;

    return FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA;
}

std::error_code exportGroupEntityIndexData(std::filesystem::path rootOutPath,
                                           foeSimulationState *pSimState) {
    if (exportIndexDataToFile(rootOutPath / entityIndexDataFilePath,
                              pSimState->groupData.persistentEntityIndices()))
        return FOE_IMEX_YAML_SUCCESS;

    return FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA;
}

std::error_code exportResources(std::filesystem::path rootOutPath,
                                foeIdGroup group,
                                foeSimulationState *pSimState) {
    // Make sure the export directory exists
    auto const dirPath = rootOutPath / resourceDirectoryPath;
    // Check if it exists already
    if (std::filesystem::exists(dirPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(dirPath)) {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    dirPath.string())
            return FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY;
        } else if (std::filesystem::is_directory(dirPath)) {
            FOE_LOG(foeImexYaml, Info,
                    "Attempting to export state via Yaml to an existing  directory at '{}'",
                    dirPath.string())
            std::filesystem::remove_all(dirPath);
        }
    }

    // Create the resource sub-directory
    std::error_code errC;

    bool created = std::filesystem::create_directories(dirPath, errC);
    if (errC) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, with error: {}",
                dirPath.string(), errC.message())
        return errC;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                dirPath.string())
        return FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION;
    } else {
        FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                dirPath.string())
    }

    // Get the valid set of resource indices
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;
    pSimState->groupData.resourceIndices(group)->exportState(maxIndices, unusedIndices);

    foeResourceID resource;
    auto unused = unusedIndices.begin();

    try {
        for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
            // Check if unused, then skip if it is
            if (unused != unusedIndices.end()) {
                if (idx == *unused)
                    continue;
            }

            resource = foeIdCreate(group, idx);

            exportResource(resource, dirPath, pSimState->pResourceNameMap, gResourceFns, pSimState);
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export resource: {} - {}", foeIdToString(resource),
                e.what());
        return FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA;
    }

    return FOE_IMEX_YAML_SUCCESS;
}

std::error_code exportComponentData(std::filesystem::path rootOutPath,
                                    foeIdGroup group,
                                    foeSimulationState *pSimState) {
    // Make sure the export directory exists
    auto const dirPath = rootOutPath / entityDirectoryPath;
    // Check if it exists already
    if (std::filesystem::exists(dirPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(dirPath)) {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    dirPath.string())
            return FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY;
        } else if (std::filesystem::is_directory(dirPath)) {
            FOE_LOG(foeImexYaml, Info,
                    "Attempting to export state via Yaml to an existing  directory at '{}'",
                    dirPath.string())
            std::filesystem::remove_all(dirPath);
        }
    }

    // Create the resource sub-directory
    std::error_code errC;

    bool created = std::filesystem::create_directories(dirPath, errC);
    if (errC) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, with error: {}",
                dirPath.string(), errC.message())
        return errC;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                dirPath.string())
        return FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION;
    } else {
        FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                dirPath.string())
    }

    // Get the valid set of entity indices
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;
    pSimState->groupData.entityIndices(group)->exportState(maxIndices, unusedIndices);

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

            exportComponents(entity, dirPath, pSimState->pEntityNameMap, gComponentFns, pSimState);
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export entity: {} - {}", foeIdToString(entity),
                e.what());
        return FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA;
    }

    return FOE_IMEX_YAML_SUCCESS;
}

} // namespace

std::error_code foeImexYamlExport(std::filesystem::path rootPath, foeSimulationState *pSimState) {
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
            return FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY;
        }
    }

    // Create the resource sub-directory
    std::error_code errC;

    bool created = std::filesystem::create_directories(rootPath, errC);
    if (errC) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, with error: {}",
                rootPath.string(), errC.message())
        return errC;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                rootPath.string())
        return errC;
    } else {
        FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                rootPath.string())
    }

    gSync.lock_shared();

    errC = exportDependencies(rootPath, pSimState);
    if (errC)
        goto EXPORT_FAILED;

    errC = exportGroupResourceIndexData(rootPath, pSimState);
    if (errC)
        goto EXPORT_FAILED;

    errC = exportGroupEntityIndexData(rootPath, pSimState);
    if (errC)
        goto EXPORT_FAILED;

    errC = exportResources(rootPath, foeIdPersistentGroup, pSimState);
    if (errC)
        goto EXPORT_FAILED;

    errC = exportComponentData(rootPath, foeIdPersistentGroup, pSimState);
    if (errC)
        goto EXPORT_FAILED;

EXPORT_FAILED:
    gSync.unlock_shared();

    return errC;
}

std::error_code foeImexYamlRegisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulationState const *)) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gResourceFns) {
        if (it == pResourceFn) {
            return FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
        }
    }

    gResourceFns.emplace_back(pResourceFn);
    return FOE_IMEX_YAML_SUCCESS;
}

std::error_code foeImexYamlDeregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulationState const *)) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gResourceFns.begin(), gResourceFns.end(), pResourceFn);
    if (searchIt != gResourceFns.end()) {
        gResourceFns.erase(searchIt);
        return FOE_IMEX_YAML_SUCCESS;
    }

    return FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED;
}

std::error_code foeImexYamlRegisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulationState const *)) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gComponentFns) {
        if (it == pComponentFn) {
            return FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
        }
    }

    gComponentFns.emplace_back(pComponentFn);
    return FOE_IMEX_YAML_SUCCESS;
}

std::error_code foeImexYamlDeregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulationState const *)) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gComponentFns.begin(), gComponentFns.end(), pComponentFn);
    if (searchIt != gComponentFns.end()) {
        gComponentFns.erase(searchIt);
        return FOE_IMEX_YAML_SUCCESS;
    }

    return FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED;
}
