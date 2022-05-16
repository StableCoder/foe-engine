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

#include "import_state.hpp"

#include <foe/delimited_string.h>
#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/group_translator.h>
#include <foe/imex/importers.hpp>
#include <foe/search_paths.hpp>
#include <foe/simulation/simulation.hpp>

#include "../log.hpp"
#include "error_code.hpp"

namespace {

std::unique_ptr<foeImporterBase> searchAndCreateImporter(std::string_view dataSetName,
                                                         foeIdGroup group,
                                                         foeSearchPaths &searchPaths) {
    auto pathReader = searchPaths.getReader();

    for (auto searchPath : *pathReader.searchPaths()) {
        if (std::filesystem::exists(searchPath / dataSetName)) {
            std::unique_ptr<foeImporterBase> importer{
                createImporter(group, searchPath / dataSetName)};
            if (importer != nullptr) {
                return importer;
            }
        }

        for (auto dirIt : std::filesystem::directory_iterator{searchPath}) {
            auto path = dirIt.path();

            if (path.stem() == dataSetName) {
                std::unique_ptr<foeImporterBase> importer{createImporter(group, path)};
                if (importer != nullptr) {
                    return importer;
                }
            }
        }
    }

    return nullptr;
}

bool generateDependencyImporters(uint32_t dependencyCount,
                                 char const **ppDependencyNames,
                                 foeIdGroup *pDependencyGroups,
                                 foeSearchPaths *pSearchPaths,
                                 std::vector<std::unique_ptr<foeImporterBase>> &importers) {
    std::vector<std::unique_ptr<foeImporterBase>> newImporters;

    auto pathReader = pSearchPaths->getReader();

    for (uint32_t i = 0; i < dependencyCount; ++i) {
        auto pImporter =
            searchAndCreateImporter(ppDependencyNames[i], pDependencyGroups[i], *pSearchPaths);

        if (pImporter != nullptr) {
            newImporters.emplace_back(std::move(pImporter));
        } else {
            FOE_LOG(General, Error, "Failed to find dataset and/or importer for dependency: {}",
                    ppDependencyNames[i])
            return false;
        }
    }

    importers = std::move(newImporters);
    return true;
}

} // namespace

auto importState(std::string_view topLevelDataSet,
                 foeSearchPaths *pSearchPaths,
                 foeSimulation **ppSimulationSet) -> std::error_code {
    foeSimulation *pTempSimSet;
    std::error_code errC = foeCreateSimulation(true, &pTempSimSet);
    if (errC)
        return errC;

    std::unique_ptr<foeSimulation, std::function<void(foeSimulation *)>> pSimulationSet{
        pTempSimSet, [](foeSimulation *ptr) { foeDestroySimulation(ptr); }};

    // Find the to-level data set, initially as if the full path were given
    std::unique_ptr<foeImporterBase> persistentImporter{
        createImporter(foeIdPersistentGroup, topLevelDataSet)};

    // If not found, try search paths
    if (persistentImporter == nullptr) {
        persistentImporter =
            searchAndCreateImporter(topLevelDataSet, foeIdPersistentGroup, *pSearchPaths);
        if (persistentImporter == nullptr) {
            FOE_LOG(General, Error,
                    "Could not find dataset and/or importer for top-level data set: {}",
                    topLevelDataSet)
            return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;
        }
    }

    // Get the list of dependencies
    uint32_t dependencyCount;
    std::vector<foeIdGroup> dependencyGroups;
    uint32_t namesLength;
    std::vector<char const *> dependencyNames;
    std::vector<char> nameArray;

    errC = persistentImporter->getDependencies(&dependencyCount, nullptr, &namesLength, nullptr);
    if (errC)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;

    dependencyGroups.resize(dependencyCount);
    nameArray.resize(namesLength);
    errC = persistentImporter->getDependencies(&dependencyCount, dependencyGroups.data(),
                                               &namesLength, nameArray.data());
    if (errC)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;

    for (uint32_t i = 0; i < dependencyCount; ++i) {
        char const *pStr = nullptr;
        foeIndexedDelimitedString(namesLength, nameArray.data(), i, &pStr);
        dependencyNames.emplace_back(pStr);
    }

    // Check for duplicate dependencies
    for (auto it = dependencyNames.begin(); it != dependencyNames.end(); ++it) {
        for (auto innerIt = it + 1; innerIt != dependencyNames.end(); ++innerIt) {
            if (strcmp(*innerIt, *it) == 0) {
                FOE_LOG(General, Error, "Duplicate dependency '{}' detected", *it)
                return FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES;
            }
        }
    }

    // Generate importers for all of the dependencies
    std::vector<std::unique_ptr<foeImporterBase>> dependencyImporters;
    bool pass =
        generateDependencyImporters(dependencyNames.size(), dependencyNames.data(),
                                    dependencyGroups.data(), pSearchPaths, dependencyImporters);
    if (!pass)
        return FOE_STATE_IMPORT_ERROR_NO_IMPORTER;

    { // Check transitive dependencies
        auto pImporter = dependencyImporters.begin();
        for (uint32_t i = 0; i < dependencyCount; ++i, ++pImporter) {
            uint32_t transitiveCount;
            uint32_t transitiveStringLength;

            errC = pImporter->get()->getDependencies(&transitiveCount, nullptr,
                                                     &transitiveStringLength, nullptr);
            if (errC)
                std::abort();

            std::vector<foeIdGroup> transitiveGroups;
            std::vector<char> transitiveNameArray;
            std::vector<char const *> transitiveNames;

            transitiveGroups.resize(transitiveCount);
            transitiveNameArray.resize(transitiveStringLength);

            errC = pImporter->get()->getDependencies(&transitiveCount, transitiveGroups.data(),
                                                     &transitiveStringLength,
                                                     transitiveNameArray.data());
            if (errC) {
                FOE_LOG(General, Error, "Failed to import sub-dependencies of the '{}' dependency",
                        dependencyNames[i])
                return FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES;
            }

            for (uint32_t i = 0; i < transitiveCount; ++i) {
                char const *pStr;
                foeIndexedDelimitedString(transitiveNameArray.size(), transitiveNameArray.data(), i,
                                          &pStr);
                transitiveNames.emplace_back(pStr);
            }

            // Check that all required transitive dependencies are available *before* it is
            // loaded, and in the correct order
            for (uint32_t j = 0; j < transitiveCount; ++j) {
                bool depFound{false};

                for (uint32_t k = 0; k < i; ++k) {
                    if (strcmp(dependencyNames[k], transitiveNames[j]) == 0) {
                        depFound = true;
                        break;
                    }
                }

                if (!depFound) {
                    FOE_LOG(General, Error,
                            "Could not find transitive dependency '{}' for dependency group '{}'",
                            transitiveNames[j], dependencyNames[i])
                    return FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED;
                }
            }
        }
    }

    { // Setup the ECS Groups
        foeIdGroup groupValue = 0;
        for (auto &it : dependencyImporters) {
            // Setup importer translations
            uint32_t srcDependencyCount;
            uint32_t srcNameArrayLength;
            errC = it->getDependencies(&srcDependencyCount, nullptr, &srcNameArrayLength, nullptr);
            if (errC)
                std::abort();

            std::vector<foeIdGroup> srcGroups;
            std::vector<char> srcNameArray;
            std::vector<char const *> srcNames;
            srcGroups.resize(srcDependencyCount);
            srcNameArray.resize(srcNameArrayLength);

            errC = it->getDependencies(&srcDependencyCount, srcGroups.data(), &srcNameArrayLength,
                                       srcNameArray.data());
            if (errC)
                std::abort();

            for (uint32_t i = 0; i < srcDependencyCount; ++i) {
                char const *pStr;
                foeIndexedDelimitedString(srcNameArray.size(), srcNameArray.data(), i, &pStr);
                srcNames.emplace_back(pStr);
            }

            srcGroups.emplace_back(foeIdPersistentGroup);
            srcNames.emplace_back("Persistent");

            std::vector<char const *> dstNames = dependencyNames;
            std::vector<foeIdGroup> dstIDs = dependencyGroups;
            dstNames.emplace_back("Persistent");
            dstIDs.emplace_back(foeIdValueToGroup(groupValue));

            foeEcsGroupTranslator newTranslator{FOE_NULL_HANDLE};
            std::error_code errC = foeEcsCreateGroupTranslator(
                srcNames.size(), srcNames.data(), srcGroups.data(), dstNames.size(),
                dstNames.data(), dstIDs.data(), &newTranslator);
            if (errC) {
                return errC;
            }

            it->setGroupTranslator(std::move(newTranslator));

            // Add to GroupData
            std::string name{it->name()};
            auto newGroupEntityIndices =
                std::make_unique<foeIdIndexGenerator>(foeIdValueToGroup(groupValue));
            auto newGroupResourceIndices =
                std::make_unique<foeIdIndexGenerator>(foeIdValueToGroup(groupValue));

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

    // Dependency Indice Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        auto *pGroupImporter = pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (pGroupImporter != nullptr) {
            pGroupImporter->getGroupResourceIndexData(
                *pSimulationSet->groupData.resourceIndices(foeIdValueToGroup(groupValue)));

            pGroupImporter->getGroupEntityIndexData(
                *pSimulationSet->groupData.entityIndices(foeIdValueToGroup(groupValue)));
        }
    }

    // Persistent Indice Data
    bool retVal = pSimulationSet->groupData.persistentImporter()->getGroupEntityIndexData(
        *pSimulationSet->groupData.persistentEntityIndices());
    if (!retVal)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA;

    retVal = pSimulationSet->groupData.persistentImporter()->getGroupResourceIndexData(
        *pSimulationSet->groupData.persistentResourceIndices());
    if (!retVal)
        return FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA;

    // Read the Resource Editor Names
    if (pSimulationSet->pResourceNameMap != nullptr) {
        // Dependent Groups
        for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
            auto *pGroupImporter =
                pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
            if (pGroupImporter == nullptr)
                continue;

            // Go through all the indexes for the group, set any available editor names
            pSimulationSet->groupData.resourceIndices(foeIdValueToGroup(groupValue))
                ->forEachID([&](foeId id) {
                    std::string editorName =
                        pGroupImporter->getResourceEditorName(foeIdGetIndex(id));

                    if (!editorName.empty())
                        pSimulationSet->pResourceNameMap->add(id, editorName);
                });
        }

        // Persistent Group
        auto *pGroupImporter = pSimulationSet->groupData.persistentImporter();

        // Go through all the indexes for the group, set any available editor names
        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycledIndexes;
        pSimulationSet->groupData.persistentResourceIndices()->forEachID([&](foeId id) {
            std::string editorName = pGroupImporter->getResourceEditorName(foeIdGetIndex(id));

            if (!editorName.empty())
                pSimulationSet->pResourceNameMap->add(id, editorName);
        });
    }

    // Importing Dependency State Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        auto *pGroupImporter = pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (pGroupImporter != nullptr) {
            if (!pGroupImporter->importStateData(pSimulationSet->pEntityNameMap,
                                                 pSimulationSet.get())) {
                return FOE_STATE_IMPORT_ERROR_NO_COMPONENT_IMPORTER;
            }
        }
    }

    // Importing Persistent State Data
    if (!pSimulationSet->groupData.persistentImporter()->importStateData(
            pSimulationSet->pEntityNameMap, pSimulationSet.get()))
        return FOE_STATE_IMPORT_ERROR_NO_COMPONENT_IMPORTER;

    // Successfully returning
    *ppSimulationSet = pSimulationSet.release();
    return FOE_STATE_IMPORT_SUCCESS;
}