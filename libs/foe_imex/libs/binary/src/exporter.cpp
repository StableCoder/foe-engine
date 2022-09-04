// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/exporter.h>

#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/result.h>
#include <foe/imex/binary/result.h>
#include <foe/imex/importer.h>
#include <foe/simulation/simulation.hpp>

#include "exporter.h"
#include "log.hpp"
#include "result.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace {

std::shared_mutex gSync;

std::vector<PFN_foeImexBinaryExportResource> gResourceFns;
std::vector<PFN_foeImexBinaryExportComponent> gComponentFns;

foeResultSet exportDependencyData(foeSimulation *pSimulation, uint32_t *pDataSize, void **pData) {
    std::vector<std::pair<foeIdGroup, char const *>> dependencies;
    uint32_t totalNameSizes = 0;

    // Get all dynamic groups
    for (uint32_t i = 0; i < foeIdNumDynamicGroups; ++i) {
        foeIdGroup groupID = foeIdValueToGroup(i);

        foeImexImporter group = pSimulation->groupData.importer(groupID);
        if (group == FOE_NULL_HANDLE)
            continue;

        char const *pGroupName;
        foeResultSet resultSet = foeImexImporterGetGroupName(group, &pGroupName);
        if (resultSet.value != FOE_SUCCESS)
            return to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_DEPENDENCIES);

        totalNameSizes += strlen(pGroupName);
        dependencies.emplace_back(std::make_pair(groupID, pGroupName));
    }

    // Allocate appropriately-sized memory
    size_t allocSize =
        dependencies.size() * (sizeof(foeIdGroup) + sizeof(uint32_t)) + totalNameSizes;
    void *pNewAlloc = malloc(allocSize);
    if (pNewAlloc == nullptr)
        return to_foeResult(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY);

    // Write out data
    uint8_t *writePtr = (uint8_t *)pNewAlloc;
    for (auto &it : dependencies) {
        // Group ID
        memcpy(writePtr, &it.first, sizeof(foeIdGroup));
        writePtr += sizeof(foeIdGroup);

        // String Length
        uint32_t strLen = strlen(it.second);
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        // String
        memcpy(writePtr, it.second, strLen);
        writePtr += strLen;
    }

    *pDataSize = allocSize;
    *pData = pNewAlloc;
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet exportIndexData(foeEcsIndexes indexes, uint32_t *pDataSize, void **pData) {
    foeResultSet result;
    foeIdIndex nextNewIndex;
    uint32_t recycledCount;

    size_t allocSize;
    void *pNewAlloc = nullptr;

    do {
        foeEcsExportIndexes(indexes, nullptr, &recycledCount, nullptr);

        // Layout is: NextNewIndex, IndexCount, Indexes
        allocSize = sizeof(foeIdIndex) + sizeof(uint32_t) + sizeof(foeIdIndex) * recycledCount;
        pNewAlloc = realloc(pNewAlloc, allocSize);
        if (pNewAlloc == nullptr)
            return to_foeResult(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY);

        result = foeEcsExportIndexes(
            indexes, &nextNewIndex, &recycledCount,
            (foeIdIndex *)((uint8_t *)pNewAlloc + sizeof(foeIdIndex) + sizeof(uint32_t)));
    } while (result.value != FOE_SUCCESS);

    // Put the next new index at the start
    memcpy(pNewAlloc, &nextNewIndex, sizeof(foeIdIndex));
    // Put total count after new index
    memcpy((uint8_t *)pNewAlloc + sizeof(foeIdIndex), &recycledCount, sizeof(uint32_t));

    *pDataSize = allocSize;
    *pData = pNewAlloc;
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet exportResource(foeResourceID resourceID,
                            char const *pResourceName,
                            foeSimulation *pSimulation,
                            std::vector<foeImexBinarySet> *pBinarySets,
                            std::vector<foeImexBinaryFiles> *pFiles) {
    if (resourceID == FOE_INVALID_ID) {
        FOE_LOG(foeImexBinary, Error, "Attempted to export resource with invalid ID");
        std::abort();
    }

    // Get the CreateInfo for a resource
    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);
    foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;

    if (resource != FOE_NULL_HANDLE) {
        resourceCI = foeResourceGetCreateInfo(resource);
    }

    if (resourceCI == FOE_NULL_HANDLE) {
        foeResultSet result =
            foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &resourceCI);
        if (result.value != FOE_SUCCESS)
            // @todo Properly handle failure to find CreateInfo
            std::abort();

        foeResourceCreateInfoIncrementRefCount(resourceCI);
    }

    bool found = false;
    for (auto const &fn : gResourceFns) {
        foeImexBinarySet set;
        foeImexBinaryFiles files;

        foeResultSet resultSet = fn(resourceCI, &set, &files);

        if (resultSet.value == FOE_SUCCESS) {
            found = true;
            pBinarySets->emplace_back(set);
            if (files.fileCount != 0)
                pFiles->emplace_back(files);
            break;
        } else if (resultSet.value < FOE_SUCCESS) {
            // A proper error occurred
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            resultSet.toString(resultSet.value, buffer);
            FOE_LOG(foeImexBinary, Error,
                    "Failed to export resource {} create info {} with error: {}", resourceID,
                    foeResourceCreateInfoGetType(resourceCI), buffer);
            found = false;
            goto EXPORT_RESOURCE_FAILED;
        }
    }

EXPORT_RESOURCE_FAILED:
    if (!found) {
        FOE_LOG(foeImexBinary, Error, "Failed to find exporter for resource {} create info type {}",
                foeIdToString(resourceID), foeResourceCreateInfoGetType(resourceCI))
        return to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE);
    } else {
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }
}

foeResultSet exportResourceData(foeIdGroup groupID,
                                foeSimulation *pSimulation,
                                std::vector<foeImexBinarySet> *pBinarySets,
                                std::vector<foeImexBinaryFiles> *pFiles) {
    // Get the valid set of resource indices
    foeResultSet result;
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;

    do {
        uint32_t count;
        foeEcsExportIndexes(pSimulation->groupData.resourceIndexes(groupID), nullptr, &count,
                            nullptr);

        unusedIndices.resize(count);
        result = foeEcsExportIndexes(pSimulation->groupData.resourceIndexes(groupID), &maxIndices,
                                     &count, unusedIndices.data());
        unusedIndices.resize(count);
    } while (result.value != FOE_SUCCESS);
    std::sort(unusedIndices.begin(), unusedIndices.end());

    foeResourceID resourceID;
    auto unused = unusedIndices.begin();

    for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
        // Check if unused, then skip if it is
        if (unused != unusedIndices.end()) {
            if (idx == *unused) {
                ++unused;
                continue;
            }
        }

        resourceID = foeIdCreate(groupID, idx);

        // Resource Name
        char *pResourceName = NULL;
        if (pSimulation->resourceNameMap != FOE_NULL_HANDLE) {
            uint32_t strLength = 0;
            foeResultSet result;

            do {
                result = foeEcsNameMapFindName(pSimulation->resourceNameMap, resourceID, &strLength,
                                               pResourceName);
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

        result = exportResource(resourceID, pResourceName, pSimulation, pBinarySets, pFiles);

        if (pResourceName)
            free(pResourceName);

        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
}

foeResultSet exportEntity(foeResourceID entityID,
                          char const *pEntityName,
                          foeSimulation *pSimulation,
                          std::vector<foeImexBinarySet> *pBinarySets) {
    for (auto const &fn : gComponentFns) {
        foeImexBinarySet set = {};

        foeResultSet result = fn(entityID, pSimulation, &set);

        if (result.value == FOE_SUCCESS) {
            // Export successful, add data to be sent out
            assert(set.pData != nullptr);
            pBinarySets->emplace_back(set);
        } else if (set.pData) {
            // Not successful and some data was given back, free it
            free(set.pData);
        }

        if (result.value < FOE_SUCCESS) {
            // Some error occurred, leave
            return result;
        }
    }

EXPORT_ENTITY_FAILED:
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet exportComponentData(foeIdGroup groupID,
                                 foeSimulation *pSimulation,
                                 std::vector<foeImexBinarySet> *pBinarySets) {
    // Get the valid set of entity indices
    foeResultSet result;
    foeIdIndex maxIndices;
    std::vector<foeIdIndex> unusedIndices;

    do {
        uint32_t count;
        foeEcsExportIndexes(pSimulation->groupData.entityIndexes(groupID), nullptr, &count,
                            nullptr);

        unusedIndices.resize(count);
        result = foeEcsExportIndexes(pSimulation->groupData.entityIndexes(groupID), &maxIndices,
                                     &count, unusedIndices.data());
        unusedIndices.resize(count);
    } while (result.value != FOE_SUCCESS);
    std::sort(unusedIndices.begin(), unusedIndices.end());

    foeEntityID entityID;
    auto unused = unusedIndices.begin();

    for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
        // Check if unused, then skip if it is
        if (unused != unusedIndices.end()) {
            if (idx == *unused) {
                ++unused;
                continue;
            }
        }

        entityID = foeIdCreate(groupID, idx);

        // Resource Name
        char *pEntityName = NULL;
        if (pSimulation->entityNameMap != FOE_NULL_HANDLE) {
            uint32_t strLength = 0;
            foeResultSet result;

            do {
                result = foeEcsNameMapFindName(pSimulation->resourceNameMap, entityID, &strLength,
                                               pEntityName);
                if (result.value == FOE_ECS_SUCCESS && pEntityName != NULL) {
                    break;
                } else if ((result.value == FOE_ECS_SUCCESS && pEntityName == NULL) ||
                           result.value == FOE_ECS_INCOMPLETE) {
                    pEntityName = (char *)realloc(pEntityName, strLength);
                    if (pEntityName == NULL)
                        std::abort();
                }
            } while (result.value != FOE_ECS_NO_MATCH);
        }

        result = exportEntity(entityID, pEntityName, pSimulation, pBinarySets);

        if (pEntityName)
            free(pEntityName);

        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
}

} // namespace

extern "C" foeResultSet foeImexBinaryExport(char const *pExportPath, foeSimulation *pSimState) {
    std::error_code errC;
    std::filesystem::path destinationPath{pExportPath};

    FOE_LOG(foeImexBinary, Verbose, "Starting process to export data out to file {}", pExportPath);

    if (std::filesystem::exists(destinationPath) &&
        !std::filesystem::is_regular_file(destinationPath)) {
        FOE_LOG(foeImexBinary, Error,
                "Attempted to export data as a binary file to an occupied non-file location '{}', "
                "unsupported",
                destinationPath.string());
        return to_foeResult(FOE_IMEX_BINARY_ERROR_DESTINATION_NOT_FILE);
    }

    // Create temporary locations

    // Lock the exporter registries
    gSync.lock_shared();
    foeResultSet resultSet;

    void *pDependencyData = nullptr;
    uint32_t dependencyDataSize;

    void *pResourceIndexData = nullptr;
    uint32_t resourceIndexDataSize;

    void *pEntityIndexData = nullptr;
    uint32_t entityIndexDataSize;

    std::vector<foeImexBinarySet> resourceDataSets;
    std::vector<foeImexBinaryFiles> resourceFiles;

    std::vector<foeImexBinarySet> componentDataSets;

    { // Dependency Data
        resultSet = exportDependencyData(pSimState, &dependencyDataSize, &pDependencyData);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Resource Index Data
        resultSet = exportIndexData(pSimState->groupData.persistentResourceIndexes(),
                                    &resourceIndexDataSize, &pResourceIndexData);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Entity Index Data
        resultSet = exportIndexData(pSimState->groupData.persistentResourceIndexes(),
                                    &entityIndexDataSize, &pEntityIndexData);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Resource Data
        resultSet =
            exportResourceData(foeIdValueToGroup(0), pSimState, &resourceDataSets, &resourceFiles);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Entity/Component Data
        resultSet = exportComponentData(foeIdPersistentGroup, pSimState, &componentDataSets);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

EXPORT_FAILED:
    gSync.unlock_shared();

    if (resultSet.value == FOE_SUCCESS) { // Open and write to file
        FILE *pOutFile = fopen(pExportPath, "wb");

        if (pOutFile == nullptr)
            goto EXPORT_WRITE_FAILED;

        size_t totalWrittenData = 0;

        fwrite(pDependencyData, dependencyDataSize, 1, pOutFile);
        totalWrittenData += dependencyDataSize;

        fwrite(pResourceIndexData, resourceIndexDataSize, 1, pOutFile);
        totalWrittenData += resourceIndexDataSize;

        fwrite(pEntityIndexData, entityIndexDataSize, 1, pOutFile);
        totalWrittenData += entityIndexDataSize;

        { // Resource Data Export
            // Create an index for Resource CreateInfo binary keys
            std::unordered_map<char const *, uint16_t> binaryKeyMap;
            for (size_t i = 0; i < resourceDataSets.size(); ++i) {
                assert(resourceDataSets[i].pKey != nullptr);

                auto searchIt = binaryKeyMap.find(resourceDataSets[i].pKey);
                if (searchIt == binaryKeyMap.end()) {
                    // Don't have it, add it
                    binaryKeyMap[resourceDataSets[i].pKey] = binaryKeyMap.size();
                }
            }

            { // Write out the binary key index
                uint16_t numBinaryKeys = binaryKeyMap.size();
                fwrite(&numBinaryKeys, sizeof(uint16_t), 1, pOutFile);
                totalWrittenData += sizeof(uint16_t);
                size_t keyLength = strlen(binaryKeyMap.begin()->first);

                for (auto const &it : binaryKeyMap) {
                    assert(keyLength == strlen(it.first));

                    fwrite(&it.second, sizeof(uint16_t), 1, pOutFile);

                    fwrite(it.first, keyLength, 1, pOutFile);

                    totalWrittenData += sizeof(uint16_t) + keyLength;
                }
            }

            // Write out resource data
            for (size_t i = 0; i < resourceDataSets.size(); ++i) {
                foeImexBinarySet &set = resourceDataSets[i];

                uint16_t binaryKeyIndex = binaryKeyMap[set.pKey];
                fwrite(&binaryKeyIndex, sizeof(uint16_t), 1, pOutFile);

                fwrite(&set.dataSize, sizeof(uint32_t), 1, pOutFile);
                fwrite(set.pData, set.dataSize, 1, pOutFile);

                totalWrittenData += sizeof(uint16_t) + sizeof(uint32_t) + set.dataSize;
            }
        }

        { // Component Data Export
            // Create an index for Entity Component binary keys
            std::unordered_map<char const *, uint16_t> binaryKeyMap;
            for (size_t i = 0; i < componentDataSets.size(); ++i) {
                assert(componentDataSets[i].pKey != nullptr);

                auto searchIt = binaryKeyMap.find(componentDataSets[i].pKey);
                if (searchIt == binaryKeyMap.end()) {
                    // Don't have it, add it
                    binaryKeyMap[componentDataSets[i].pKey] = binaryKeyMap.size();
                }
            }

            { // Write out the binary key index
                uint16_t numBinaryKeys = binaryKeyMap.size();
                fwrite(&numBinaryKeys, sizeof(uint16_t), 1, pOutFile);
                totalWrittenData += sizeof(uint16_t);
                size_t keyLength = strlen(binaryKeyMap.begin()->first);

                for (auto const &it : binaryKeyMap) {
                    assert(keyLength == strlen(it.first));

                    fwrite(&it.second, sizeof(uint16_t), 1, pOutFile);

                    fwrite(it.first, keyLength, 1, pOutFile);

                    totalWrittenData += sizeof(uint16_t) + keyLength;
                }
            }

            // Write out component data
            for (size_t i = 0; i < componentDataSets.size(); ++i) {
                foeImexBinarySet &set = componentDataSets[i];

                uint16_t binaryKeyIndex = binaryKeyMap[set.pKey];
                fwrite(&binaryKeyIndex, sizeof(uint16_t), 1, pOutFile);

                fwrite(&set.dataSize, sizeof(uint32_t), 1, pOutFile);
                fwrite(set.pData, set.dataSize, 1, pOutFile);

                totalWrittenData += sizeof(uint16_t) + sizeof(uint32_t) + set.dataSize;
            }
        }

        { // External Resource Content
            std::vector<std::string> externalFiles;
            for (auto const &it : resourceFiles) {
                for (size_t i = 0; i < it.fileCount; ++i) {
                    externalFiles.emplace_back(it.ppFiles[i]);
                }
            }
            std::sort(externalFiles.begin(), externalFiles.end());
            auto newEndIt = std::unique(externalFiles.begin(), externalFiles.end());
            externalFiles.erase(newEndIt, externalFiles.end());

            struct FileExport {
                std::string filePath;
                foeManagedMemory content;
                void *pData;
                uint32_t dataSize;
            };
            std::vector<FileExport> fileExportList;
            size_t fileExportIndexSize = 0;

            for (auto const &it : externalFiles) {
                foeManagedMemory managedMemory = FOE_NULL_HANDLE;
                foeResultSet result =
                    pSimState->groupData.findExternalFile(it.c_str(), &managedMemory);
                if (result.value != FOE_SUCCESS) {
                    std::abort();
                }

                void *pData;
                uint32_t dataSize;
                foeManagedMemoryGetData(managedMemory, &pData, &dataSize);

                fileExportList.emplace_back(FileExport{
                    .filePath = it,
                    .content = managedMemory,
                    .pData = pData,
                    .dataSize = dataSize,
                });
                fileExportIndexSize +=
                    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + it.size();
            }

            size_t totalWritten = ftell(pOutFile);
            assert(totalWrittenData == totalWritten);

            // Print out file index locations
            uint32_t totalFileOffset = totalWritten + fileExportIndexSize;
            for (auto const &it : fileExportList) {
                // Data offset and size
                fwrite(&totalFileOffset, sizeof(uint32_t), 1, pOutFile);
                fwrite(&it.dataSize, sizeof(uint32_t), 1, pOutFile);

                // File path
                uint32_t strLen = it.filePath.size();
                fwrite(&strLen, sizeof(uint32_t), 1, pOutFile);
                fwrite(it.filePath.data(), strLen, 1, pOutFile);

                totalFileOffset += it.dataSize;
            }

            // Write out raw file data
            for (auto const &it : fileExportList) {
                fwrite(it.pData, it.dataSize, 1, pOutFile);

                foeManagedMemoryDecrementUse(it.content);
            }
        }

        fclose(pOutFile);
    }

EXPORT_WRITE_FAILED:

    for (auto const &it : componentDataSets) {
        if (it.pData)
            free(it.pData);
    }
    for (auto const &it : resourceFiles) {
        if (it.ppFiles)
            free(it.ppFiles);
    }
    for (auto const &it : resourceDataSets) {
        if (it.pData)
            free(it.pData);
    }
    if (pEntityIndexData)
        free(pEntityIndexData);
    if (pResourceIndexData)
        free(pResourceIndexData);
    if (pDependencyData)
        free(pDependencyData);

    if (resultSet.value == FOE_SUCCESS)
        FOE_LOG(foeImexBinary, Verbose, "Successfully exported data to file {}", pExportPath);

    return resultSet;
}

extern "C" foeResultSet foeImexBinaryRegisterResourceExportFn(
    PFN_foeImexBinaryExportResource exportResourceFn) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gResourceFns) {
        if (it == exportResourceFn) {
            return to_foeResult(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);
        }
    }

    gResourceFns.emplace_back(exportResourceFn);
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

extern "C" foeResultSet foeImexBinaryDeregisterResourceExportFn(
    PFN_foeImexBinaryExportResource exportResourceFn) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gResourceFns.begin(), gResourceFns.end(), exportResourceFn);
    if (searchIt != gResourceFns.end()) {
        gResourceFns.erase(searchIt);
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }

    return to_foeResult(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED);
}

extern "C" foeResultSet foeImexBinaryRegisterComponentExportFn(
    PFN_foeImexBinaryExportComponent exportComponentFn) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gComponentFns) {
        if (it == exportComponentFn) {
            return to_foeResult(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);
        }
    }

    gComponentFns.emplace_back(exportComponentFn);
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

extern "C" foeResultSet foeImexBinaryDeregisterComponentExportFn(
    PFN_foeImexBinaryExportComponent exportComponentFn) {
    std::scoped_lock lock{gSync};

    auto searchIt = std::find(gComponentFns.begin(), gComponentFns.end(), exportComponentFn);
    if (searchIt != gComponentFns.end()) {
        gComponentFns.erase(searchIt);
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }

    return to_foeResult(FOE_IMEX_BINARY_ERROR_FUNCTIONALITY_NOT_REGISTERED);
}