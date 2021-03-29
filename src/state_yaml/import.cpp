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

#include <foe/ecs/entity_id.hpp>
#include <foe/ecs/groups.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/log.hpp>
#include <foe/search_paths.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <string_view>

#include "entity.hpp"
#include "import.hpp"
#include "translation.hpp"

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view indexDataFilePath = "index_data.yml";
constexpr std::string_view resourcesDirectoryPath = "resources";
constexpr std::string_view stateDataDirectoryPath = "state_data";

struct StateDataDependency {
    std::string name;
    foeGroupID group;
    std::filesystem::path path;
};

bool importStateDependenciesFromNode(YAML::Node const &dependenciesNode,
                                     std::vector<StateDataDependency> &stateDependencies) {
    try {
        for (auto it = dependenciesNode.begin(); it != dependenciesNode.end(); ++it) {
            StateDataDependency newDependency;

            yaml_read_required("name", *it, newDependency.name);
            yaml_read_required("group_id", *it, newDependency.group);

            stateDependencies.emplace_back(newDependency);
        }
    } catch (YAML::Exception const &e) {
        FOE_LOG(General, Error, "dependencies::{}", e.what())
        return false;
    }

    return true;
}

bool importStateDependenciesFromFile(std::filesystem::path filePath,
                                     std::vector<StateDataDependency> &stateDependencies) {
    std::ifstream dependencyYamlFile{filePath, std::ifstream::in};
    if (!dependencyYamlFile) {
        FOE_LOG(General, Error, "Could not open Yaml state dependencies file at '{}'",
                filePath.native())
        return false;
    }

    YAML::Node dependenciesNode;
    try {
        dependenciesNode = YAML::Load(dependencyYamlFile);
    } catch (YAML::Exception const &e) {
        FOE_LOG(General, Error, "Could not parse dependencies file at '{}': {}", filePath.native(),
                e.what())
        return false;
    }

    return importStateDependenciesFromNode(dependenciesNode, stateDependencies);
}

bool findGroupState(std::string_view groupName,
                    foeSearchPaths &searchPaths,
                    std::filesystem::path &groupStatePath) {

    auto reader = searchPaths.getReader();

    for (auto const &it : *reader.searchPaths()) {
        // Check for a folder by the same name with a dependencies.yml file inside
        std::filesystem::path temp = it / groupName;
        if (std::filesystem::is_directory(temp) &&
            std::filesystem::is_regular_file(temp / dependenciesFilePath)) {
            groupStatePath = temp;
            return true;
        }
    }

    return false;
}

bool checkGroupStateDependencies(std::string_view groupName,
                                 foeSearchPaths &searchPaths,
                                 std::vector<StateDataDependency> &stateDependencies) {
    std::string_view dependenciesFilePath = "dependencies.yml";

    // Check for duplicates
    for (auto it = stateDependencies.begin(); it != stateDependencies.end(); ++it) {
        for (auto innerIt = it + 1; innerIt != stateDependencies.end(); ++innerIt) {
            if (innerIt->name == it->name) {
                FOE_LOG(General, Error, "Duplicate dependency '{}' detected for group state: {}",
                        innerIt->name, groupName)
                return false;
            }
        }
    }

    // Find all the dependent group state files/folders
    for (auto &it : stateDependencies) {
        if (!findGroupState(it.name, searchPaths, it.path)) {
            FOE_LOG(General, Error, "Could not find dependency '{}' for group state '{}'", it.name,
                    groupName)
            return false;
        }
    }

    // Check transitive dependencies
    for (auto dependencyIt = stateDependencies.begin(); dependencyIt != stateDependencies.end();
         ++dependencyIt) {
        std::vector<StateDataDependency> transitiveDependencies;
        bool imported = importStateDependenciesFromFile(dependencyIt->path / dependenciesFilePath,
                                                        transitiveDependencies);
        if (!imported) {
            FOE_LOG(General, Error,
                    "Failed to import state dependencies of dependency group '{}' for top-level "
                    "group '{}'",
                    dependencyIt->name, groupName)
            return false;
        }

        // Check that all required transitive dependencies are available *before* it is loaded,
        // and in the correct order
        auto checkIt = stateDependencies.begin();
        for (auto transIt : transitiveDependencies) {
            bool depFound{false};

            for (; checkIt != dependencyIt; ++checkIt) {
                if (checkIt->name == transIt.name) {
                    depFound = true;
                    break;
                }
            }

            if (!depFound) {
                FOE_LOG(General, Error,
                        "Could not find transitive dependency '{}' for dependency group '{}' for "
                        "top-level group '{}'",
                        transIt.name, dependencyIt->name, groupName)
                return false;
            }
        }
    }

    return true;
}

bool importGroupStateIndexDataFromFile(std::string_view groupName,
                                       std::filesystem::path filePath,
                                       foeEcsIndexGenerator &ecsGroup) {
    std::ifstream yamlFile{filePath, std::ifstream::in};
    if (!yamlFile) {
        FOE_LOG(General, Error, "Could not open Yaml group index data file at '{}'",
                filePath.native())
        return false;
    }

    YAML::Node node;
    try {
        node = YAML::Load(yamlFile);
    } catch (YAML::Exception const &e) {
        FOE_LOG(General, Error, "Could not parse group index data file file at '{}': {}",
                filePath.native(), e.what())
        return false;
    }

    try {
        yaml_read_index_generator(node, ecsGroup);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to parse Group State Index Data from Yaml file: {}",
                e.what())
        return false;
    }

    return true;
}

bool generateGroupTranslations(std::vector<StateDataDependency> const &dependencies,
                               foeEcsGroups &ecsGroups,
                               std::vector<GroupTranslation> &translationList) {
    translationList.reserve(dependencies.size());

    for (auto const &it : dependencies) {
        auto *targetGroup = ecsGroups.group(it.name);

        if (targetGroup == nullptr)
            return false;

        translationList.emplace_back(GroupTranslation{
            .source = it.group,
            .target = targetGroup->groupID(),
        });
    }

    return true;
}

bool importStateDataFromFile(std::filesystem::path filePath,
                             foeGroupID targetGroup,
                             std::vector<GroupTranslation> groupTranslations,
                             StatePools &statePools,
                             ResourcePools &resourcePools) {
    if (!std::filesystem::is_regular_file(filePath)) {
        FOE_LOG(General, Error,
                "Attempted to parse '{}' as a state data file when it's NOT a regular file!",
                filePath.native())
        return false;
    }

    std::ifstream yamlFile{filePath, std::ifstream::in};
    if (!yamlFile) {
        FOE_LOG(General, Error, "Could not open Yaml group state data file at '{}'",
                filePath.native())
        return false;
    }

    YAML::Node node;
    try {
        node = YAML::Load(yamlFile);
    } catch (YAML::Exception const &e) {
        FOE_LOG(General, Error,
                "Could not parse the Yaml in the group state data file file at '{}': {}",
                filePath.native(), e.what())
        return false;
    }

    try {
        auto entity =
            yaml_read_entity(node, targetGroup, groupTranslations, &statePools, &resourcePools);
        if (entity == FOE_INVALID_ENTITY) {
            return false;
        } else {
            FOE_LOG(General, Info, "Successfully parsed entity {}", foeEntityID_to_string(entity))
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to parse Yaml state data for '{}': {}", filePath.native(),
                e.what())
        return false;
    }

    return true;
}

bool importGroupStateData(foeGroupID targetGroup,
                          std::filesystem::path targetGroupPath,
                          foeEcsGroups &ecsGroups,
                          StatePools &statePools,
                          ResourcePools &resourcePools) {
    std::vector<StateDataDependency> groupDependencies;
    bool retVal =
        importStateDependenciesFromFile(targetGroupPath / dependenciesFilePath, groupDependencies);
    if (!retVal) {
        return false;
    }

    std::vector<GroupTranslation> groupTranslations;
    retVal = generateGroupTranslations(groupDependencies, ecsGroups, groupTranslations);
    if (!retVal) {
        return false;
    }

    // Make sure there's state data directory (can be empty if new)
    if (!std::filesystem::exists(targetGroupPath / stateDataDirectoryPath)) {
        FOE_LOG(General, Warning, "For target group '{}', no state data found",
                ecsGroups.group(targetGroup)->name())
        return true;
    }

    // Go through all entries in the state data directory, recursively
    for (auto &dirEntry :
         std::filesystem::recursive_directory_iterator{targetGroupPath / stateDataDirectoryPath}) {
        FOE_LOG(General, Info, "Visiting: {}", dirEntry.path().native());
        if (std::filesystem::is_directory(dirEntry))
            continue;

        if (!std::filesystem::is_regular_file(dirEntry)) {
            FOE_LOG(General, Warning,
                    "State data directory entry '{}' not a directory or regular file! Possible "
                    "corruption!",
                    dirEntry.path().native())
            return false;
        }

        // Otherwise, parse the entity state data
        retVal = importStateDataFromFile(dirEntry.path(), targetGroup, groupTranslations,
                                         statePools, resourcePools);
        if (!retVal)
            return false;
    }

    return true;
}

} // namespace

bool importGroupState(std::filesystem::path yamlPath,
                      foeSearchPaths &searchPaths,
                      foeEcsGroups &ecsGroups,
                      StatePools &statePools,
                      ResourcePools &resourcePools) {
    // Determines if the given item is a directory or a
    // file, as how we parse depends on it
    if (!std::filesystem::is_directory(yamlPath)) {
        FOE_LOG(General, Error,
                "Given YAML state path '{}' is not a "
                "directory, not supported",
                yamlPath.native())
        return false;
    }

    std::string groupName = yamlPath.stem().native();

    // Group Dependencies
    std::vector<StateDataDependency> groupDependencies;
    std::string_view dependenciesFilePath = "dependencies.yml";
    bool retVal =
        importStateDependenciesFromFile(yamlPath / dependenciesFilePath, groupDependencies);
    if (!retVal)
        return false;

    retVal = checkGroupStateDependencies(groupName, searchPaths, groupDependencies);
    if (!retVal)
        return false;

    foeGroupID newGroupID = 0;
    for (auto const &it : groupDependencies) {
        auto newGroup =
            std::make_unique<foeEcsIndexGenerator>(it.name, foeEcsNormalizedToGroupID(it.group));
        auto success = ecsGroups.addGroup(std::move(newGroup));
        if (!success) {
            FOE_LOG(General, Error, "Could not create entity group '{}'", it.name);
            return false;
        }
        ++newGroupID;
    }

    // Group Index Data
    retVal = importGroupStateIndexDataFromFile(groupName, yamlPath / indexDataFilePath,
                                               *ecsGroups.persistentGroup());
    if (!retVal)
        return false;

    // Dependency State Data
    for (auto const &it : groupDependencies) {
        retVal = importGroupStateData(it.group, it.path, ecsGroups, statePools, resourcePools);
        if (!retVal)
            return false;
    }

    // Group State Data
    retVal = importGroupStateData(foeEcsGroups::Persistent, yamlPath, ecsGroups, statePools,
                                  resourcePools);
    if (!retVal)
        return false;

    return true;
}