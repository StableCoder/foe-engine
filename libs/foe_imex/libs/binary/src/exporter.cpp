// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/exporter.h>

#include <foe/ecs/binary.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/ecs/result.h>
#include <foe/imex/binary/result.h>
#include <foe/imex/importer.h>
#include <foe/simulation/simulation.hpp>

#include "binary_file_header.h"
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

struct ResourceSet {
    foeResourceID id;
    uint32_t offset;
    uint32_t dataSets;
};

struct EntitySet {
    foeEntityID id;
    uint32_t dataSets;
};

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
    size_t allocSize = sizeof(uint32_t) +
                       dependencies.size() * (sizeof(foeIdGroup) + sizeof(uint32_t)) +
                       totalNameSizes;
    void *pNewAlloc = malloc(allocSize);
    if (pNewAlloc == nullptr)
        return to_foeResult(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY);

    // Write out data
    uint8_t *writePtr = (uint8_t *)pNewAlloc;

    { // Number of dependencies
        uint32_t numDependencies = dependencies.size();
        memcpy(writePtr, &numDependencies, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);
    }

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
                            uint32_t currentOffset,
                            uint32_t *pResourceSize,
                            foeSimulation *pSimulation,
                            std::vector<ResourceSet> *pResourceSets,
                            std::vector<foeImexBinarySet> *pBinarySets,
                            std::vector<foeImexBinaryFiles> *pFiles) {
    if (resourceID == FOE_INVALID_ID) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR, "Attempted to export resource with invalid ID");
        std::abort();
    }

    // Get the CreateInfo for a resource
    foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
    foeResultSet result = foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &resourceCI);
    if (result.value != FOE_SUCCESS || resourceCI == FOE_NULL_HANDLE)
        // @todo Properly handle failure to find CreateInfo
        std::abort();

    bool found = false;
    for (auto const &fn : gResourceFns) {
        foeImexBinarySet set;
        foeImexBinaryFiles files;

        foeResultSet resultSet = fn(resourceCI, &set, &files);

        if (resultSet.value == FOE_SUCCESS) {
            found = true;

            ResourceSet newSet = {
                .id = resourceID,
                .offset = currentOffset,
                .dataSets = 1,
            };

            pResourceSets->emplace_back(std::move(newSet));

            pBinarySets->emplace_back(set);
            if (files.fileCount != 0)
                pFiles->emplace_back(files);

            uint32_t resourceDataSize = 0;
            resourceDataSize += sizeof(uint32_t); // Data Set Count
            resourceDataSize += sizeof(uint16_t); // Key Index
            resourceDataSize += sizeof(uint32_t); // CI Data Size
            resourceDataSize += set.dataSize;     // CI Data
            *pResourceSize = resourceDataSize;

            break;
        } else if (resultSet.value < FOE_SUCCESS) {
            // A proper error occurred
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            resultSet.toString(resultSet.value, buffer);
            FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                    "Failed to export resource {} create info {} with error: {}", resourceID,
                    foeResourceCreateInfoGetType(resourceCI), buffer);
            found = false;
            goto EXPORT_RESOURCE_FAILED;
        }
    }

EXPORT_RESOURCE_FAILED:
    if (resourceCI)
        foeResourceCreateInfoDecrementRefCount(resourceCI);
    if (!found) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Failed to find exporter for resource {} create info type {}",
                foeIdToString(resourceID), foeResourceCreateInfoGetType(resourceCI))
        return to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_EXPORT_RESOURCE);
    } else {
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }
}

foeResultSet exportResourceData(foeIdGroup groupID,
                                foeSimulation *pSimulation,
                                std::vector<ResourceSet> *pResourceSets,
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
    uint32_t totalResourceDataSize = 0;

    for (foeIdIndex idx = foeIdIndexMinValue; idx < maxIndices; ++idx) {
        // Check if unused, then skip if it is
        if (unused != unusedIndices.end()) {
            if (idx == *unused) {
                ++unused;
                continue;
            }
        }

        resourceID = foeIdCreate(groupID, idx);

        uint32_t dataSize;
        result = exportResource(resourceID, totalResourceDataSize, &dataSize, pSimulation,
                                pResourceSets, pBinarySets, pFiles);
        totalResourceDataSize += dataSize;

        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
}

foeResultSet exportEntity(foeEntityID entityID,
                          foeSimulation *pSimulation,
                          std::vector<EntitySet> *pEntitySets,
                          std::vector<foeImexBinarySet> *pBinarySets) {
    size_t exportedComponents = 0;

    for (auto const &fn : gComponentFns) {
        foeImexBinarySet set = {};

        foeResultSet result = fn(entityID, pSimulation, &set);

        if (result.value == FOE_SUCCESS) {
            // Export successful, add data to be sent out
            assert(set.pData != nullptr);
            pBinarySets->emplace_back(set);
            ++exportedComponents;
        } else if (set.pData) {
            // Not successful and some data was given back, free it
            free(set.pData);
        }

        if (result.value < FOE_SUCCESS) {
            // Some error occurred, leave
            return result;
        }
    }

    EntitySet newSet = {
        .id = entityID,
        .dataSets = (uint32_t)exportedComponents,
    };

    pEntitySets->emplace_back(std::move(newSet));

    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet exportComponentData(foeIdGroup groupID,
                                 foeSimulation *pSimulation,
                                 std::vector<EntitySet> *pEntitySets,
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

        result = exportEntity(entityID, pSimulation, pEntitySets, pBinarySets);

        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
}

void binary_write_EditorNames(foeIdGroup groupID,
                              foeIdIndex maxIndexID,
                              foeIdIndex *pUnusedIndexes,
                              uint32_t unusedIndexCount,
                              foeEcsNameMap nameMap,
                              uint32_t *pOffset,
                              uint32_t *pNamesWritten,
                              uint32_t *pWriteSize,
                              FILE *pWriteFile) {
    foeIdIndex *pUnused = pUnusedIndexes;
    foeIdIndex *const pEndUnusedIndex = pUnusedIndexes + unusedIndexCount;

    uint32_t numNames = 0;
    uint32_t maxNameLength = 0;
    uint32_t editorNameDataOffset = 0;

    // Write out indexes and offsets
    *pOffset = ftell(pWriteFile);

    // Write out indexes
    for (foeIdIndex indexID = foeIdIndexMinValue; indexID < maxIndexID; ++indexID) {
        // Check if this particular index is unused, skip if it is
        if (pUnused != pEndUnusedIndex && *pUnused == indexID) {
            ++pUnused;
            continue;
        }

        foeId fullID = foeIdCreate(groupID, indexID);

        uint32_t nameLength;
        foeResultSet result = foeEcsNameMapFindName(nameMap, fullID, &nameLength, nullptr);
        if (result.value == FOE_ECS_NO_MATCH) {
            continue;
        }
        if (nameLength > maxNameLength) {
            maxNameLength = nameLength;
        }

        // Write out the index
        fwrite(&indexID, sizeof(foeIdIndex), 1, pWriteFile);

        // Write out the offset of the string in the data section
        fwrite(&editorNameDataOffset, sizeof(uint32_t), 1, pWriteFile);

        editorNameDataOffset += sizeof(uint32_t) + nameLength;
        ++numNames;
    }

    *pNamesWritten = numNames;
    if (numNames == 0) {
        *pWriteSize = 0;
        return;
    }

    // Write out the actual editor names
    std::unique_ptr<char[]> nameBuffer(new char[maxNameLength]);
    pUnused = pUnusedIndexes;
    for (foeIdIndex indexID = foeIdIndexMinValue; indexID < maxIndexID; ++indexID) {
        // Check if this particular index is unused, skip if it is
        if (pUnused != pEndUnusedIndex && *pUnused == indexID) {
            ++pUnused;
            continue;
        }

        foeId fullID = foeIdCreate(groupID, indexID);

        uint32_t nameLength = maxNameLength;
        foeResultSet result = foeEcsNameMapFindName(nameMap, fullID, &nameLength, nameBuffer.get());
        if (result.value != FOE_SUCCESS) {
            std::abort();
        }

        fwrite(&nameLength, sizeof(uint32_t), 1, pWriteFile);
        fwrite(nameBuffer.get(), nameLength, 1, pWriteFile);
    }

    *pWriteSize = (numNames * (sizeof(foeIdIndex) + sizeof(uint32_t))) + editorNameDataOffset;
}

} // namespace

extern "C" foeResultSet foeImexBinaryExport(char const *pExportPath, foeSimulation *pSimState) {
    std::error_code errC;
    std::filesystem::path destinationPath{pExportPath};

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_VERBOSE, "Starting process to export data out to file {}",
            pExportPath);

    if (std::filesystem::exists(destinationPath) &&
        !std::filesystem::is_regular_file(destinationPath)) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
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
    uint32_t dependencyDataSize = 0;

    void *pResourceIndexData = nullptr;
    uint32_t resourceIndexDataSize = 0;

    void *pEntityIndexData = nullptr;
    uint32_t entityIndexDataSize = 0;

    std::vector<ResourceSet> resourceSets;
    std::vector<foeImexBinarySet> resourceDataSets;
    std::vector<foeImexBinaryFiles> resourceFiles;

    std::vector<EntitySet> entitySets;
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
        resultSet = exportIndexData(pSimState->groupData.persistentEntityIndexes(),
                                    &entityIndexDataSize, &pEntityIndexData);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Resource Data
        resultSet = exportResourceData(foeIdPersistentGroup, pSimState, &resourceSets,
                                       &resourceDataSets, &resourceFiles);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

    { // Entity/Component Data
        resultSet =
            exportComponentData(foeIdPersistentGroup, pSimState, &entitySets, &componentDataSets);
        if (resultSet.value != FOE_SUCCESS)
            goto EXPORT_FAILED;
    }

EXPORT_FAILED:
    gSync.unlock_shared();

    if (resultSet.value == FOE_SUCCESS) { // Open and write to file
        BinaryFileHeader fileHeaderData = {};
        FILE *pOutFile = fopen(pExportPath, "wb");

        if (pOutFile == nullptr)
            goto EXPORT_WRITE_FAILED;

        size_t totalWrittenData = 0;

        fwrite(&fileHeaderData, sizeof(BinaryFileHeader), 1, pOutFile);
        totalWrittenData += sizeof(BinaryFileHeader);

        fileHeaderData.dependencyDataOffset = ftell(pOutFile);
        fwrite(pDependencyData, dependencyDataSize, 1, pOutFile);
        totalWrittenData += dependencyDataSize;

        fileHeaderData.resourceIndexDataOffset = ftell(pOutFile);
        fileHeaderData.resourceIndexDataSize = resourceIndexDataSize;
        fwrite(pResourceIndexData, resourceIndexDataSize, 1, pOutFile);
        totalWrittenData += resourceIndexDataSize;

        fileHeaderData.entityIndexDataOffset = ftell(pOutFile);
        fileHeaderData.entityIndexDataSize = entityIndexDataSize;
        fwrite(pEntityIndexData, entityIndexDataSize, 1, pOutFile);
        totalWrittenData += entityIndexDataSize;

        { // Write out Resource Editor Names
            foeResultSet result;
            foeIdIndex maxIndex;
            std::vector<foeIdIndex> unusedIndices;

            do {
                uint32_t count;
                foeEcsExportIndexes(pSimState->groupData.persistentResourceIndexes(), nullptr,
                                    &count, nullptr);

                unusedIndices.resize(count);
                result = foeEcsExportIndexes(pSimState->groupData.persistentResourceIndexes(),
                                             &maxIndex, &count, unusedIndices.data());
                unusedIndices.resize(count);
            } while (result.value != FOE_SUCCESS);
            std::sort(unusedIndices.begin(), unusedIndices.end());

            uint32_t writtenBytes = 0;
            binary_write_EditorNames(
                foeIdPersistentGroup, maxIndex, unusedIndices.data(), unusedIndices.size(),
                pSimState->resourceNameMap, &fileHeaderData.resourceEditorNamesOffset,
                &fileHeaderData.numResourceEditorNames, &writtenBytes, pOutFile);
            totalWrittenData += writtenBytes;
        }

        { // Write out Entity Editor Names
            foeResultSet result;
            foeIdIndex maxIndex;
            std::vector<foeIdIndex> unusedIndices;

            do {
                uint32_t count;
                foeEcsExportIndexes(pSimState->groupData.persistentEntityIndexes(), nullptr, &count,
                                    nullptr);

                unusedIndices.resize(count);
                result = foeEcsExportIndexes(pSimState->groupData.persistentEntityIndexes(),
                                             &maxIndex, &count, unusedIndices.data());
                unusedIndices.resize(count);
            } while (result.value != FOE_SUCCESS);
            std::sort(unusedIndices.begin(), unusedIndices.end());

            uint32_t writtenBytes = 0;
            binary_write_EditorNames(foeIdPersistentGroup, maxIndex, unusedIndices.data(),
                                     unusedIndices.size(), pSimState->entityNameMap,
                                     &fileHeaderData.entityEditorNamesOffset,
                                     &fileHeaderData.numEntityEditorNames, &writtenBytes, pOutFile);
            totalWrittenData += writtenBytes;
        }

        { // Resource Data Export
            // Create an index for Resource CreateInfo binary keys
            fileHeaderData.resourceBinaryKeyIndexOffset = ftell(pOutFile);
            std::unordered_map<char const *, uint16_t> binaryKeyMap;
            for (size_t i = 0; i < resourceDataSets.size(); ++i) {
                assert(resourceDataSets[i].pKey != nullptr);

                auto searchIt = binaryKeyMap.find(resourceDataSets[i].pKey);
                if (searchIt == binaryKeyMap.end()) {
                    // Don't have it, add it
                    binaryKeyMap[resourceDataSets[i].pKey] = binaryKeyMap.size();
                }
            }

            // Write out the binary key index
            uint16_t numBinaryKeys = binaryKeyMap.size();
            fwrite(&numBinaryKeys, sizeof(uint16_t), 1, pOutFile);
            totalWrittenData += sizeof(uint16_t);

            if (!binaryKeyMap.empty()) {
                uint16_t keyLength = strlen(binaryKeyMap.begin()->first);
                fwrite(&keyLength, sizeof(uint16_t), 1, pOutFile);
                totalWrittenData += sizeof(uint16_t);

                for (auto const &it : binaryKeyMap) {
                    assert(keyLength == strlen(it.first));

                    fwrite(&it.second, sizeof(uint16_t), 1, pOutFile);

                    fwrite(it.first, keyLength, 1, pOutFile);

                    totalWrittenData += sizeof(uint16_t) + keyLength;
                }

                // Write out resource index
                fileHeaderData.resourceIndexOffset = ftell(pOutFile);
                for (size_t i = 0; i < resourceSets.size(); ++i) {
                    auto const &set = resourceSets[i];

                    // ResourceID
                    uint8_t buffer[8];
                    uint32_t bufSize = sizeof(buffer);
                    auto result = binary_write_foeResourceID(set.id, &bufSize, buffer);
                    if (result.value != FOE_SUCCESS)
                        std::abort();

                    fwrite(buffer, bufSize, 1, pOutFile);
                    totalWrittenData += bufSize;

                    // Resource Data Offset
                    fwrite(&set.offset, sizeof(uint32_t), 1, pOutFile);
                    totalWrittenData += sizeof(uint32_t);
                }

                // Write out resource data
                fileHeaderData.resourceDataOffset = ftell(pOutFile);
                auto dataIt = resourceDataSets.begin();
                for (size_t i = 0; i < resourceSets.size(); ++i) {
                    auto const &set = resourceSets[i];

                    // ResourceCIs
                    fwrite(&set.dataSets, sizeof(uint32_t), 1, pOutFile);
                    totalWrittenData += sizeof(uint32_t);

                    for (uint32_t j = 0; j < set.dataSets; ++j) {
                        uint16_t binaryKeyIndex = binaryKeyMap[dataIt->pKey];
                        fwrite(&binaryKeyIndex, sizeof(uint16_t), 1, pOutFile);

                        fwrite(&dataIt->dataSize, sizeof(uint32_t), 1, pOutFile);
                        fwrite(dataIt->pData, dataIt->dataSize, 1, pOutFile);

                        totalWrittenData += sizeof(uint16_t) + sizeof(uint32_t) + dataIt->dataSize;

                        ++dataIt;
                    }
                }
            }
        }

        fileHeaderData.entityDataOffset = ftell(pOutFile);
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

            // Write out the binary key index
            uint16_t numBinaryKeys = binaryKeyMap.size();
            fwrite(&numBinaryKeys, sizeof(uint16_t), 1, pOutFile);
            totalWrittenData += sizeof(uint16_t);

            if (!binaryKeyMap.empty()) {
                uint16_t keyLength = strlen(binaryKeyMap.begin()->first);
                fwrite(&keyLength, sizeof(uint16_t), 1, pOutFile);
                totalWrittenData += sizeof(uint16_t);

                for (auto const &it : binaryKeyMap) {
                    assert(keyLength == strlen(it.first));

                    fwrite(&it.second, sizeof(uint16_t), 1, pOutFile);

                    fwrite(it.first, keyLength, 1, pOutFile);

                    totalWrittenData += sizeof(uint16_t) + keyLength;
                }

                // Write out component data
                auto dataIt = componentDataSets.begin();
                for (size_t i = 0; i < entitySets.size(); ++i) {
                    auto const &set = entitySets[i];

                    // EntityID
                    uint8_t buffer[8];
                    uint32_t bufSize = sizeof(buffer);
                    auto result = binary_write_foeResourceID(set.id, &bufSize, buffer);
                    if (result.value != FOE_SUCCESS)
                        std::abort();

                    fwrite(buffer, bufSize, 1, pOutFile);
                    totalWrittenData += bufSize;

                    // Component Data
                    fwrite(&set.dataSets, sizeof(uint32_t), 1, pOutFile);
                    totalWrittenData += sizeof(uint32_t);

                    for (uint32_t j = 0; j < set.dataSets; ++j) {
                        uint16_t binaryKeyIndex = binaryKeyMap[dataIt->pKey];
                        fwrite(&binaryKeyIndex, sizeof(uint16_t), 1, pOutFile);

                        fwrite(&dataIt->dataSize, sizeof(uint32_t), 1, pOutFile);
                        fwrite(dataIt->pData, dataIt->dataSize, 1, pOutFile);

                        totalWrittenData += sizeof(uint16_t) + sizeof(uint32_t) + dataIt->dataSize;

                        ++dataIt;
                    }
                }
            }
        }

        fileHeaderData.fileDataOffset = ftell(pOutFile);
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
                // Size of each entry is fileOffset + dataSize + strLen + str
                fileExportIndexSize +=
                    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + it.size();
            }

            size_t totalWritten = ftell(pOutFile);
            assert(totalWrittenData == totalWritten);

            // Print out the number of file index/data sets we have
            uint32_t numFiles = fileExportList.size();
            fwrite(&numFiles, sizeof(uint32_t), 1, pOutFile);

            // Print out file index locations
            uint32_t totalFileOffset = totalWritten + sizeof(uint32_t) + fileExportIndexSize;
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

        // Write the file header at the beginning of the file
        fseek(pOutFile, 0, SEEK_SET);
        fwrite(&fileHeaderData, sizeof(BinaryFileHeader), 1, pOutFile);

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
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_VERBOSE, "Successfully exported data to file {}",
                pExportPath);

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