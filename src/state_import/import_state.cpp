// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "import_state.hpp"

#include <foe/delimited_string.h>
#include <foe/ecs/group_translator.h>
#include <foe/ecs/name_map.h>
#include <foe/imex/importer.h>
#include <foe/search_paths.hpp>
#include <foe/simulation/simulation.hpp>

#include "../log.hpp"
#include "result.h"

namespace {

foeImexImporter searchAndCreateImporter(std::string_view dataSetName,
                                        foeIdGroup group,
                                        foeSearchPaths &searchPaths) {
    auto pathReader = searchPaths.getReader();

    for (auto searchPath : *pathReader.searchPaths()) {
        if (std::filesystem::exists(searchPath / dataSetName)) {
            foeImexImporter importer = createImporter(
                group, std::filesystem::path{searchPath / dataSetName}.string().c_str());
            if (importer != FOE_NULL_HANDLE) {
                return importer;
            }
        }

        for (auto dirIt : std::filesystem::directory_iterator{searchPath}) {
            auto path = dirIt.path();

            if (path.stem() == dataSetName) {
                foeImexImporter importer = createImporter(group, path.string().c_str());
                if (importer != FOE_NULL_HANDLE) {
                    return importer;
                }
            }
        }
    }

    return FOE_NULL_HANDLE;
}

bool generateDependencyImporters(uint32_t dependencyCount,
                                 char const **ppDependencyNames,
                                 foeIdGroup *pDependencyGroups,
                                 foeSearchPaths *pSearchPaths,
                                 std::vector<foeImexImporter> &importers) {
    std::vector<foeImexImporter> newImporters;

    auto pathReader = pSearchPaths->getReader();

    for (uint32_t i = 0; i < dependencyCount; ++i) {
        foeImexImporter importer =
            searchAndCreateImporter(ppDependencyNames[i], pDependencyGroups[i], *pSearchPaths);

        if (importer != FOE_NULL_HANDLE) {
            newImporters.emplace_back(importer);
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

foeResultSet importState(std::string_view topLevelDataSet,
                         foeSearchPaths *pSearchPaths,
                         foeSimulation **ppSimulationSet) {
    foeSimulation *pTempSimSet;
    foeResultSet result = foeCreateSimulation(true, &pTempSimSet);
    if (result.value != FOE_SUCCESS)
        return result;

    std::unique_ptr<foeSimulation, std::function<void(foeSimulation *)>> pSimulationSet{
        pTempSimSet, [](foeSimulation *ptr) { foeDestroySimulation(ptr); }};

    // Find the to-level data set, initially as if the full path were given
    foeImexImporter persistentImporter =
        createImporter(foeIdPersistentGroup, std::string{topLevelDataSet}.c_str());

    // If not found, try search paths
    if (persistentImporter == FOE_NULL_HANDLE) {
        persistentImporter =
            searchAndCreateImporter(topLevelDataSet, foeIdPersistentGroup, *pSearchPaths);
        if (persistentImporter == FOE_NULL_HANDLE) {
            FOE_LOG(General, Error,
                    "Could not find dataset and/or importer for top-level data set: {}",
                    topLevelDataSet)
            return to_foeResult(FOE_STATE_IMPORT_ERROR_NO_IMPORTER);
        }
    }

    // Get the list of dependencies
    uint32_t dependencyCount;
    std::vector<foeIdGroup> dependencyGroups;
    uint32_t namesLength;
    std::vector<char const *> dependencyNames;
    std::vector<char> nameArray;

    result = foeImexImporterGetDependencies(persistentImporter, &dependencyCount, nullptr,
                                            &namesLength, nullptr);
    if (result.value != FOE_SUCCESS)
        return to_foeResult(FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES);

    dependencyGroups.resize(dependencyCount);
    nameArray.resize(namesLength);
    result =
        foeImexImporterGetDependencies(persistentImporter, &dependencyCount,
                                       dependencyGroups.data(), &namesLength, nameArray.data());
    if (result.value != FOE_SUCCESS)
        return to_foeResult(FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES);

    for (uint32_t i = 0; i < dependencyCount; ++i) {
        char const *pStr = nullptr;
        foeIndexedDelimitedString(namesLength, nameArray.data(), i, '\0', nullptr, &pStr);
        dependencyNames.emplace_back(pStr);
    }

    // Check for duplicate dependencies
    for (auto it = dependencyNames.begin(); it != dependencyNames.end(); ++it) {
        for (auto innerIt = it + 1; innerIt != dependencyNames.end(); ++innerIt) {
            if (strcmp(*innerIt, *it) == 0) {
                FOE_LOG(General, Error, "Duplicate dependency '{}' detected", *it)
                return to_foeResult(FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES);
            }
        }
    }

    // Generate importers for all of the dependencies
    std::vector<foeImexImporter> dependencyImporters;
    bool pass =
        generateDependencyImporters(dependencyNames.size(), dependencyNames.data(),
                                    dependencyGroups.data(), pSearchPaths, dependencyImporters);
    if (!pass)
        return to_foeResult(FOE_STATE_IMPORT_ERROR_NO_IMPORTER);

    { // Check transitive dependencies
        auto importerIt = dependencyImporters.begin();
        for (uint32_t i = 0; i < dependencyCount; ++i, ++importerIt) {
            uint32_t transitiveCount;
            uint32_t transitiveStringLength;

            result = foeImexImporterGetDependencies(*importerIt, &transitiveCount, nullptr,
                                                    &transitiveStringLength, nullptr);
            if (result.value != FOE_SUCCESS)
                std::abort();

            std::vector<foeIdGroup> transitiveGroups;
            std::vector<char> transitiveNameArray;
            std::vector<char const *> transitiveNames;

            transitiveGroups.resize(transitiveCount);
            transitiveNameArray.resize(transitiveStringLength);

            result = foeImexImporterGetDependencies(
                *importerIt, &transitiveCount, transitiveGroups.data(), &transitiveStringLength,
                transitiveNameArray.data());
            if (result.value != FOE_SUCCESS) {
                FOE_LOG(General, Error, "Failed to import sub-dependencies of the '{}' dependency",
                        dependencyNames[i])
                return to_foeResult(FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES);
            }

            for (uint32_t i = 0; i < transitiveCount; ++i) {
                char const *pStr;
                foeIndexedDelimitedString(transitiveNameArray.size(), transitiveNameArray.data(), i,
                                          '\0', nullptr, &pStr);
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
                    return to_foeResult(FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED);
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
            result = foeImexImporterGetDependencies(it, &srcDependencyCount, nullptr,
                                                    &srcNameArrayLength, nullptr);
            if (result.value != FOE_SUCCESS)
                std::abort();

            std::vector<foeIdGroup> srcGroups;
            std::vector<char> srcNameArray;
            std::vector<char const *> srcNames;
            srcGroups.resize(srcDependencyCount);
            srcNameArray.resize(srcNameArrayLength);

            result = foeImexImporterGetDependencies(it, &srcDependencyCount, srcGroups.data(),
                                                    &srcNameArrayLength, srcNameArray.data());
            if (result.value != FOE_SUCCESS)
                std::abort();

            for (uint32_t i = 0; i < srcDependencyCount; ++i) {
                char const *pStr;
                foeIndexedDelimitedString(srcNameArray.size(), srcNameArray.data(), i, '\0',
                                          nullptr, &pStr);
                srcNames.emplace_back(pStr);
            }

            srcGroups.emplace_back(foeIdPersistentGroup);
            srcNames.emplace_back("Persistent");

            std::vector<char const *> dstNames = dependencyNames;
            std::vector<foeIdGroup> dstIDs = dependencyGroups;
            dstNames.emplace_back("Persistent");
            dstIDs.emplace_back(foeIdValueToGroup(groupValue));

            foeEcsGroupTranslator newTranslator{FOE_NULL_HANDLE};
            result = foeEcsCreateGroupTranslator(srcNames.size(), srcNames.data(), srcGroups.data(),
                                                 dstNames.size(), dstNames.data(), dstIDs.data(),
                                                 &newTranslator);
            if (result.value != FOE_SUCCESS) {
                return result;
            }

            foeImexImporterSetGroupTranslator(it, newTranslator);

            // Add to GroupData
            char const *pGroupName;
            foeResultSet result = foeImexImporterGetGroupName(it, &pGroupName);
            if (result.value != FOE_SUCCESS) {
                return to_foeResult(FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE);
            }

            foeEcsIndexes newGroupEntityIndexes = FOE_NULL_HANDLE;
            foeEcsIndexes newGroupResourceIndexes = FOE_NULL_HANDLE;

            result = foeEcsCreateIndexes(foeIdValueToGroup(groupValue), &newGroupEntityIndexes);
            if (result.value != FOE_SUCCESS)
                return to_foeResult(FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE);

            result = foeEcsCreateIndexes(foeIdValueToGroup(groupValue), &newGroupResourceIndexes);
            if (result.value != FOE_SUCCESS) {
                foeEcsDestroyIndexes(newGroupEntityIndexes);
                return to_foeResult(FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE);
            }

            auto success = pSimulationSet->groupData.addDynamicGroup(newGroupEntityIndexes,
                                                                     newGroupResourceIndexes, it);
            if (!success) {
                FOE_LOG(General, Error, "Could not setup Group '{}'", pGroupName);
                return to_foeResult(FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE);
            }
            ++groupValue;
        }

        pSimulationSet->groupData.setPersistentImporter(persistentImporter);
    }

    // Dependency Indice Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        foeImexImporter groupImporter =
            pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (groupImporter == FOE_NULL_HANDLE)
            continue;

        foeImexImporterGetGroupResourceIndexData(
            groupImporter,
            pSimulationSet->groupData.resourceIndexes(foeIdValueToGroup(groupValue)));

        foeImexImporterGetGroupEntityIndexData(
            groupImporter, pSimulationSet->groupData.entityIndexes(foeIdValueToGroup(groupValue)));
    }

    // Persistent Indice Data
    result =
        foeImexImporterGetGroupEntityIndexData(pSimulationSet->groupData.persistentImporter(),
                                               pSimulationSet->groupData.persistentEntityIndexes());
    if (result.value != FOE_SUCCESS)
        return to_foeResult(FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA);

    result = foeImexImporterGetGroupResourceIndexData(
        pSimulationSet->groupData.persistentImporter(),
        pSimulationSet->groupData.persistentResourceIndexes());
    if (result.value != FOE_SUCCESS)
        return to_foeResult(FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA);

    // Read the Resource Editor Names
    if (pSimulationSet->resourceNameMap != FOE_NULL_HANDLE) {
        // Dependent Groups
        for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
            foeImexImporter groupImporter =
                pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
            if (groupImporter == FOE_NULL_HANDLE)
                continue;

            struct CallContext {
                foeImexImporter importer;
                foeEcsNameMap nameMap;
            };
            CallContext callContext = {
                .importer = groupImporter,
                .nameMap = pSimulationSet->resourceNameMap,
            };

            // Go through all the indexes for the group, set any available editor names
            foeEcsForEachID(
                pSimulationSet->groupData.resourceIndexes(foeIdValueToGroup(groupValue)),
                [](void *pContext, foeId id) {
                    CallContext *pCallContext = (CallContext *)pContext;

                    uint32_t nameLength;
                    foeResultSet result = foeImexImporterGetResourceEditorName(
                        pCallContext->importer, foeIdGetIndex(id), &nameLength, NULL);
                    if (result.value == FOE_SUCCESS && nameLength > 0) {
                        std::string editorName;
                        do {
                            editorName.resize(nameLength);
                            result = foeImexImporterGetResourceEditorName(
                                pCallContext->importer, foeIdGetIndex(id), &nameLength,
                                editorName.data());
                        } while (result.value != FOE_SUCCESS);

                        foeEcsNameMapAdd(pCallContext->nameMap, id, editorName.c_str());
                    }
                },
                &callContext);
        }

        // Persistent Group
        foeImexImporter groupImporter = pSimulationSet->groupData.persistentImporter();

        // Go through all the indexes for the group, set any available editor names
        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycledIndexes;

        struct CallContext {
            foeImexImporter importer;
            foeEcsNameMap nameMap;
        };
        CallContext callContext = {
            .importer = groupImporter,
            .nameMap = pSimulationSet->resourceNameMap,
        };

        foeEcsForEachID(
            pSimulationSet->groupData.persistentResourceIndexes(),
            [](void *pContext, foeId id) {
                CallContext *pCallContext = (CallContext *)pContext;

                uint32_t nameLength;
                foeResultSet result = foeImexImporterGetResourceEditorName(
                    pCallContext->importer, foeIdGetIndex(id), &nameLength, NULL);
                if (result.value == FOE_SUCCESS && nameLength > 0) {
                    std::string editorName;
                    do {
                        editorName.resize(nameLength);
                        result = foeImexImporterGetResourceEditorName(
                            pCallContext->importer, foeIdGetIndex(id), &nameLength,
                            editorName.data());
                    } while (result.value != FOE_SUCCESS);

                    foeEcsNameMapAdd(pCallContext->nameMap, id, editorName.c_str());
                }
            },
            &callContext);
    }

    // Import Resource History/Records
    {
        // Dynamic Groups
        for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
            foeImexImporter groupImporter =
                pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
            if (groupImporter == FOE_NULL_HANDLE)
                continue;

            // Go through all GroupIDs upto this group, and import any resource data for all of it
            for (foeIdGroupValue resourceGroupValue = 0; resourceGroupValue <= groupValue;
                 ++resourceGroupValue) {
                auto *pGroupIndexes = pSimulationSet->groupData.resourceIndexes(
                    foeIdValueToGroup(resourceGroupValue));
                if (pGroupIndexes == nullptr)
                    continue;

                struct CallContext {
                    foeImexImporter importer;
                    foeResourceRecords records;
                    foeIdGroupValue groupValue;
                };
                CallContext callContext = {
                    .importer = groupImporter,
                    .records = pSimulationSet->resourceRecords,
                    .groupValue = groupValue,
                };

                // Go through all the indexes for the group, set any available editor names
                foeEcsForEachID(
                    pSimulationSet->groupData.resourceIndexes(foeIdValueToGroup(groupValue)),
                    [](void *pContext, foeId id) {
                        CallContext *pCallContext = (CallContext *)pContext;

                        foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
                        foeResultSet result = foeImexImporterGetResourceCreateInfo(
                            pCallContext->importer, id, &resourceCI);
                        if (result.value != FOE_SUCCESS)
                            return;

                        if (foeIdGroupToValue(foeIdGetGroup(id)) == pCallContext->groupValue) {
                            foeResourceAddRecordEntry(pCallContext->records, id);
                        }

                        if (resourceCI != FOE_NULL_HANDLE) {
                            foeResourceAddSavedRecord(pCallContext->records,
                                                      foeIdValueToGroup(pCallContext->groupValue),
                                                      id, resourceCI);
                            foeResourceCreateInfoDecrementRefCount(resourceCI);
                        }
                    },
                    &callContext);
            }
        }

        // Persistent Group
        foeImexImporter persistentImporter = pSimulationSet->groupData.persistentImporter();

        // Go through all GroupIDs upto this group, and import any resource data for all of it
        for (foeIdGroupValue resourceGroupValue = 0;
             resourceGroupValue <= foeIdPersistentGroupValue; ++resourceGroupValue) {
            auto *pGroupIndexes =
                pSimulationSet->groupData.resourceIndexes(foeIdValueToGroup(resourceGroupValue));
            if (pGroupIndexes == nullptr)
                continue;

            struct CallContext {
                foeImexImporter importer;
                foeResourceRecords records;
            };
            CallContext callContext = {
                .importer = persistentImporter,
                .records = pSimulationSet->resourceRecords,
            };

            // Go through all the indexes for the group, set any available editor names
            foeEcsForEachID(
                pSimulationSet->groupData.resourceIndexes(
                    foeIdValueToGroup(foeIdPersistentGroupValue)),
                [](void *pContext, foeId id) {
                    CallContext *pCallContext = (CallContext *)pContext;

                    foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
                    foeResultSet result = foeImexImporterGetResourceCreateInfo(
                        pCallContext->importer, id, &resourceCI);
                    if (result.value != FOE_SUCCESS)
                        return;

                    if (foeIdGroupToValue(foeIdGetGroup(id)) == foeIdPersistentGroupValue) {
                        foeResourceAddRecordEntry(pCallContext->records, id);
                    }

                    if (resourceCI != FOE_NULL_HANDLE) {
                        foeResourceAddSavedRecord(pCallContext->records,
                                                  foeIdValueToGroup(foeIdPersistentGroupValue), id,
                                                  resourceCI);
                        foeResourceCreateInfoDecrementRefCount(resourceCI);
                    }
                },
                &callContext);
        }
    }

    // Importing Dependency State Data
    for (foeIdGroup groupValue = 0; groupValue < foeIdNumDynamicGroups; ++groupValue) {
        foeImexImporter groupImporter =
            pSimulationSet->groupData.importer(foeIdValueToGroup(groupValue));
        if (groupImporter == FOE_NULL_HANDLE)
            continue;

        result = foeImexImporterGetStateData(groupImporter, pSimulationSet->entityNameMap,
                                             pSimulationSet.get());
        if (result.value != FOE_SUCCESS)
            return result;
    }

    // Importing Persistent State Data
    result = foeImexImporterGetStateData(pSimulationSet->groupData.persistentImporter(),
                                         pSimulationSet->entityNameMap, pSimulationSet.get());
    if (result.value != FOE_SUCCESS)
        return result;

    for (auto &it : pSimulationSet->componentPools) {
        if (it.pMaintenanceFn) {
            it.pMaintenanceFn(it.pComponentPool);
        }
    }

    // Successfully returning
    *ppSimulationSet = pSimulationSet.release();
    return to_foeResult(FOE_STATE_IMPORT_SUCCESS);
}