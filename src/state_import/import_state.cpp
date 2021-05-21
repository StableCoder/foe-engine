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

#include "import_state.hpp"

#include <foe/ecs/group_translator.hpp>
#include <foe/imex/importers.hpp>
#include <foe/search_paths.hpp>

#include "../log.hpp"
#include "../simulation_set.hpp"
#include "error_code.hpp"

namespace {

bool generateDependencyImporters(std::vector<foeIdGroupValueNameSet> const &dependencies,
                                 foeSearchPaths *pSearchPaths,
                                 std::vector<std::unique_ptr<foeImporterBase>> &importers) {
    std::vector<std::unique_ptr<foeImporterBase>> newImporters;

    auto pathReader = pSearchPaths->getReader();

    for (auto const &depIt : dependencies) {
        for (auto searchPath : *pathReader.searchPaths()) {
            for (auto dirIt : std::filesystem::directory_iterator{searchPath}) {
                auto path = dirIt.path();

                if (path.stem() == depIt.name) {
                    std::unique_ptr<foeImporterBase> importer{
                        createImporter(foeIdValueToGroup(depIt.groupValue), path)};
                    if (importer != nullptr) {
                        newImporters.emplace_back(std::move(importer));
                        goto DEPENDENCY_FOUND;
                    } else {
                        FOE_LOG(General, Error, "Failed to find importer for dependency at: {}",
                                path.string())
                        return false;
                    }
                }
            }
        }
    DEPENDENCY_FOUND:;
    }

    importers = std::move(newImporters);
    return true;
}

} // namespace

auto importState(std::filesystem::path stateDataPath,
                 foeSearchPaths *pSearchPaths,
                 SimulationSet **ppSimulationSet) -> std::error_code {
    auto pSimulationSet = std::make_unique<SimulationSet>();

    // Find the importer for the starting path
    std::unique_ptr<foeImporterBase> persistentImporter{
        createImporter(foeIdPersistentGroup, stateDataPath)};
    if (persistentImporter == nullptr) {
        FOE_LOG(General, Error, "Could not find importer for state data at path: {}",
                stateDataPath.string())
        return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;
    }

    // Get the list of dependencies
    std::vector<foeIdGroupValueNameSet> dependencies;
    bool pass = persistentImporter->getDependencies(dependencies);
    if (!pass)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;

    // Check for duplicate dependencies
    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
        for (auto innerIt = it + 1; innerIt != dependencies.end(); ++innerIt) {
            if (innerIt->name == it->name) {
                FOE_LOG(General, Error, "Duplicate dependency '{}' detected")
                return FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES;
            }
        }
    }

    // Generate importers for all of the dependencies
    std::vector<std::unique_ptr<foeImporterBase>> dependencyImporters;
    pass = generateDependencyImporters(dependencies, pSearchPaths, dependencyImporters);
    if (!pass)
        return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;

    { // Check transitive dependencies
        auto pImporter = dependencyImporters.begin();
        for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ++depIt, ++pImporter) {
            std::vector<foeIdGroupValueNameSet> transitiveDependencies;
            pass = pImporter->get()->getDependencies(transitiveDependencies);
            if (!pass) {
                FOE_LOG(General, Error, "Failed to import sub-dependencies of the '{}' dependency",
                        depIt->name)
                return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;
            }

            // Check that all required transitive dependencies are available *before* it is loaded,
            // and in the correct order
            auto checkIt = dependencies.begin();
            for (auto const &transIt : transitiveDependencies) {
                bool depFound{false};

                for (; checkIt != depIt; ++checkIt) {
                    if (checkIt->name == transIt.name) {
                        depFound = true;
                        break;
                    }
                }

                if (!depFound) {
                    FOE_LOG(General, Error,
                            "Could not find transitive dependency '{}' for dependency group '{}'",
                            transIt.name, depIt->name)
                    return FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED;
                }
            }
        }
    }

    { // Setup the ECS Groups
        foeIdGroup groupValue = 0;
        for (auto &it : dependencyImporters) {
            // Setup importer translations
            std::vector<foeIdGroupValueNameSet> groupDependencies;
            it->getDependencies(groupDependencies);

            foeIdGroupTranslator newTranslator;
            auto errC = foeIdCreateTranslator(groupDependencies, dependencies, &newTranslator);

            it->setGroupTranslator(std::move(newTranslator));

            // Add to GroupData
            std::string name{it->name()};
            auto newGroupEntityIndices =
                std::make_unique<foeIdIndexGenerator>(name, foeIdValueToGroup(groupValue));
            auto newGroupResourceIndices =
                std::make_unique<foeIdIndexGenerator>(name, foeIdValueToGroup(groupValue));

            auto success = pSimulationSet->groupData.addDynamicGroup(
                std::move(newGroupEntityIndices), std::move(newGroupResourceIndices),
                std::move(it));
            if (!success) {
                FOE_LOG(General, Error, "Could not setup Group '{}'", name);
                return FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE;
            }
            ++groupValue;
        }

        pSimulationSet->groupData.setPersistentImporter(std::move(persistentImporter));
    }

    // Persistent Group Index Data
    bool retVal = pSimulationSet->groupData.persistentImporter()->getGroupEntityIndexData(
        *pSimulationSet->groupData.persistentEntityIndices());
    if (!retVal)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA;

    retVal = pSimulationSet->groupData.persistentImporter()->getGroupResourceIndexData(
        *pSimulationSet->groupData.persistentResourceIndices());
    if (!retVal)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA;

    // Load Persistent resource definitions
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        auto *pGroupImporter = pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (pGroupImporter != nullptr) {
            pGroupImporter->importResourceDefinitions(&pSimulationSet->resourceNameMap,
                                                      pSimulationSet->resourceLoaders2,
                                                      pSimulationSet->resourcePools);
        }
    }

    // Load dependency resource definitions
    retVal = pSimulationSet->groupData.persistentImporter()->importResourceDefinitions(
        &pSimulationSet->resourceNameMap, pSimulationSet->resourceLoaders2,
        pSimulationSet->resourcePools);
    if (!retVal)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE;

    // Importing Dependency State Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        auto *pGroupImporter = pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (pGroupImporter != nullptr) {
            pGroupImporter->importStateData(&pSimulationSet->state);
        }
    }

    // Importing Persistent State Data
    pSimulationSet->groupData.persistentImporter()->importStateData(&pSimulationSet->state);

    // Successfully returning
    *ppSimulationSet = pSimulationSet.release();
    return FOE_STATE_IMPORT_SUCCESS;
}