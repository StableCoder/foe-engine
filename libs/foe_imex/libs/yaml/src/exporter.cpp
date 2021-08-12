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

#include <foe/imex/yaml/exporter.hpp>

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/imex/exporters.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "log.hpp"

#include <fstream>

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view resourceIndexDataFilePath = "resource_index_data.yml";
constexpr std::string_view resourceDirectoryPath = "resources";
constexpr std::string_view entityIndexDataFilePath = "entity_index_data.yml";
constexpr std::string_view entityDirectoryPath = "entities";

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

void exportResource(foeResourceID resource,
                    std::filesystem::path directory,
                    foeEditorNameMap *pNameMap,
                    std::vector<std::vector<foeKeyYamlPair> (*)(
                        foeResourceID, foeResourcePoolBase **, uint32_t)> const &resourceFns,
                    foeResourcePoolBase **ppResourcePools,
                    uint32_t resourcePoolCount) {
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
        auto keyDataPairs = fn(resource, ppResourcePools, resourcePoolCount);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    emitYaml(directory / std::string{id_to_filename(resource, editorName) + ".yml"}, rootNode);
}

void exportComponents(foeEntityID entity,
                      std::filesystem::path directory,
                      foeEditorNameMap *pNameMap,
                      std::vector<std::vector<foeKeyYamlPair> (*)(
                          foeEntityID, foeComponentPoolBase **, uint32_t)> const &componentFns,
                      foeComponentPoolBase **ppComponentPools,
                      uint32_t componentPoolCount) {
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
        auto keyDataPairs = fn(entity, ppComponentPools, componentPoolCount);

        for (auto const &it : keyDataPairs) {
            rootNode[it.key] = it.data;
        }
    }

    emitYaml(directory / std::string{id_to_filename(entity, editorName) + ".yml"}, rootNode);
}

} // namespace

foeYamlExporter::foeYamlExporter() { foeRegisterExporter(this); }

foeYamlExporter::~foeYamlExporter() { foeDeregisterExporter(this); }

bool foeYamlExporter::exportState(std::filesystem::path rootPath, foeSimulationState *pSimState) {
    // Make sure the export directory exists

    // Check if it exists already
    if (std::filesystem::exists(rootPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(rootPath)) {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    rootPath.string())
            return false;
        } else if (std::filesystem::is_directory(rootPath)) {
            FOE_LOG(foeImexYaml, Info,
                    "Attempting to export state via Yaml to an existing  directory at '{}'",
                    rootPath.string())
            std::filesystem::remove_all(rootPath);
        }
    }

    // Create the resource sub-directory
    std::error_code errC;

    bool created = std::filesystem::create_directories(rootPath, errC);
    if (errC) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, with error: {}",
                rootPath.string(), errC.message())
        return false;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                rootPath.string())
        return false;
    } else {
        FOE_LOG(foeImexYaml, Info, "Created new directory at '{}' to export state as Yaml",
                rootPath.string())
    }

    if (!exportDependencies(rootPath, pSimState))
        return false;
    if (!exportGroupResourceIndexData(rootPath, pSimState))
        return false;
    if (!exportGroupEntityIndexData(rootPath, pSimState))
        return false;
    if (!exportResources(rootPath, 0, pSimState))
        return false;
    if (!exportComponentData(rootPath, 0, pSimState))
        return false;

    return true;
}

bool foeYamlExporter::registerResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeResourcePoolBase **, uint32_t)) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mResourceFns) {
        if (it == pResourceFn) {
            return false;
        }
    }

    mResourceFns.emplace_back(pResourceFn);
    return true;
}

void foeYamlExporter::deregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeResourcePoolBase **, uint32_t)) {
    std::scoped_lock lock{mSync};

    auto searchIt = std::find(mResourceFns.begin(), mResourceFns.end(), pResourceFn);
    if (searchIt != mResourceFns.end()) {
        mResourceFns.erase(searchIt);
    }
}

bool foeYamlExporter::registerComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeComponentPoolBase **, uint32_t)) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mComponentFns) {
        if (it == pComponentFn) {
            return false;
        }
    }

    mComponentFns.emplace_back(pComponentFn);
    return true;
}

void foeYamlExporter::deregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeComponentPoolBase **, uint32_t)) {
    std::scoped_lock lock{mSync};

    auto searchIt = std::find(mComponentFns.begin(), mComponentFns.end(), pComponentFn);
    if (searchIt != mComponentFns.end()) {
        mComponentFns.erase(searchIt);
    }
}

bool foeYamlExporter::exportDependencies(std::filesystem::path rootOutPath,
                                         foeSimulationState *pSimState) {
    bool retVal = true;
    mSync.lock_shared();

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
        retVal = false;
    }

    mSync.unlock_shared();
    return retVal;
}

bool foeYamlExporter::exportGroupResourceIndexData(std::filesystem::path rootOutPath,
                                                   foeSimulationState *pSimState) {
    mSync.lock_shared();
    bool retVal = exportIndexDataToFile(rootOutPath / resourceIndexDataFilePath,
                                        pSimState->groupData.persistentResourceIndices());
    mSync.unlock_shared();
    return retVal;
}

bool foeYamlExporter::exportGroupEntityIndexData(std::filesystem::path rootOutPath,
                                                 foeSimulationState *pSimState) {
    mSync.lock_shared();
    bool retVal = exportIndexDataToFile(rootOutPath / entityIndexDataFilePath,
                                        pSimState->groupData.persistentResourceIndices());
    mSync.unlock_shared();
    return retVal;
}

bool foeYamlExporter::exportResources(std::filesystem::path rootOutPath,
                                      foeIdGroup group,
                                      foeSimulationState *pSimState) {
    bool retVal = true;
    mSync.lock_shared();

    // Make sure the export directory exists
    auto const dirPath = rootOutPath / resourceDirectoryPath;
    // Check if it exists already
    if (std::filesystem::exists(dirPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(dirPath)) {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    dirPath.string())
            return false;
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
        return false;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                dirPath.string())
        return false;
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

            exportResource(resource, dirPath, pSimState->pResourceNameMap, mResourceFns,
                           pSimState->resourcePools.data(),
                           static_cast<uint32_t>(pSimState->resourcePools.size()));
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export resource: {} - {}", foeIdToString(resource),
                e.what());
        retVal = false;
    }

    mSync.unlock_shared();
    return retVal;
}

bool foeYamlExporter::exportComponentData(std::filesystem::path rootOutPath,
                                          foeIdGroup group,
                                          foeSimulationState *pSimState) {
    bool retVal = true;
    mSync.lock_shared();

    // Make sure the export directory exists
    auto const dirPath = rootOutPath / entityDirectoryPath;
    // Check if it exists already
    if (std::filesystem::exists(dirPath)) {
        // Determine if it's a file or a directory
        if (std::filesystem::is_regular_file(dirPath)) {
            FOE_LOG(foeImexYaml, Error,
                    "Attempted to export state data as a single Yaml file '{}', unsupported",
                    dirPath.string())
            return false;
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
        return false;
    } else if (!created) {
        FOE_LOG(foeImexYaml, Error,
                "Failed to create directory '{}' to export Yaml state, no error given",
                dirPath.string())
        return false;
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

            exportComponents(entity, dirPath, pSimState->pEntityNameMap, mComponentFns,
                             pSimState->componentPools.data(),
                             static_cast<uint32_t>(pSimState->componentPools.size()));
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeImexYaml, Error, "Failed to export entity: {} - {}", foeIdToString(entity),
                e.what());
        retVal = false;
    }

    mSync.unlock_shared();
    return retVal;
}