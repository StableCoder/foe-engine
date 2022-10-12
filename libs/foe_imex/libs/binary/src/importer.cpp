// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/importer.h>

#include <foe/ecs/binary.h>
#include <foe/imex/binary/type_defs.h>
#include <foe/imex/type_defs.h>
#include <foe/memory_mapped_file.h>

#include "binary_file_header.h"
#include "importer_functions.hpp"
#include "log.hpp"
#include "result.h"

#include <cassert>
#include <filesystem>
#include <string_view>

namespace {

struct foeBinaryImporter {
    foeStructureType sType;
    void *pNext;

    std::filesystem::path path;
    std::string name;
    foeIdGroup group;
    foeEcsGroupTranslator groupTranslator;

    foeManagedMemory memoryMappedFile;
    std::byte *pFileData;
    uint32_t fileSize;
    BinaryFileHeader fileHeader;

    std::map<uint32_t, std::string_view> resourceKeyMap;
};

FOE_DEFINE_HANDLE_CASTS(importer, foeBinaryImporter, foeImexImporter)

std::map<uint32_t, std::string_view> getKeyMap(std::byte const *pData, uint32_t *pReadSize) {
    std::map<uint32_t, std::string_view> keyMap;

    uint16_t keyCount = *(uint16_t const *)pData;
    pData += sizeof(uint16_t);

    if (keyCount == 0) {
        if (pReadSize)
            *pReadSize = sizeof(uint16_t);
        return keyMap;
    }

    uint16_t keyLength = *(uint16_t const *)pData;
    pData += sizeof(uint16_t);

    for (uint16_t i = 0; i < keyCount; ++i) {
        uint16_t index = *(uint16_t const *)pData;
        pData += sizeof(uint16_t);

        keyMap[index] = std::string_view{(char const *)pData, keyLength};
        pData += keyLength;
    }

    if (pReadSize)
        *pReadSize =
            sizeof(uint16_t) + sizeof(uint16_t) + (keyCount * (sizeof(uint16_t) + keyLength));
    return keyMap;
}

void destroy(foeImexImporter importer) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_VERBOSE, "[{}] foeBinaryImporter - Destroying ({})",
            (void *)pImporter, pImporter->name.c_str());

    foeManagedMemoryDecrementUse(pImporter->memoryMappedFile);

    if (pImporter->groupTranslator != FOE_NULL_HANDLE)
        foeEcsDestroyGroupTranslator(pImporter->groupTranslator);

    pImporter->~foeBinaryImporter();

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_VERBOSE, "[{}] foeBinaryImporter - Destroyed",
            (void *)pImporter);
    free(pImporter);
}

foeResultSet getGroupID(foeImexImporter importer, foeIdGroup *pGroupID) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    *pGroupID = pImporter->group;
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet getGroupName(foeImexImporter importer, char const **ppGroupName) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    *ppGroupName = pImporter->name.c_str();
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet setGroupTranslator(foeImexImporter importer, foeEcsGroupTranslator groupTranslator) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    pImporter->groupTranslator = groupTranslator;

    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet getDependencies(foeImexImporter importer,
                             uint32_t *pDependencyCount,
                             foeIdGroup *pDependencyGroups,
                             uint32_t *pNamesLength,
                             char *pNames) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);
    size_t bytesLeft =
        pImporter->fileHeader.resourceIndexDataOffset - pImporter->fileHeader.dependencyDataOffset;

    std::byte const *pData = pImporter->pFileData + pImporter->fileHeader.dependencyDataOffset;

    uint32_t dependencyCount = *(uint32_t const *)pData;
    pData += sizeof(uint32_t);
    bytesLeft -= sizeof(uint32_t);

    // Figure out names length and allocate space for the names and GroupID lists
    uint32_t namesLength =
        bytesLeft - dependencyCount * (sizeof(foeIdGroup) + sizeof(uint32_t)) + dependencyCount;

    // If the destination buffers were not provided, just return the required size to fit all the
    // content
    if (pDependencyGroups == nullptr && pNames == nullptr) {
        *pDependencyCount = dependencyCount;
        *pNamesLength = namesLength;

        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }

    foeResultSet result = to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    if (*pDependencyCount < dependencyCount || *pNamesLength < namesLength)
        result = to_foeResult(FOE_IMEX_BINARY_INCOMPLETE);

    // In case we can't fit all the data in the destination buffers, determine the max for each that
    // we can
    char *const pEndName = pNames + *pNamesLength;
    uint32_t const processedCount = std::min(*pDependencyCount, (uint32_t)dependencyCount);

    for (uint32_t i = 0; i < processedCount; ++i) {
        *(pDependencyGroups + i) = *(foeIdGroup const *)pData;
        pData += sizeof(foeIdGroup);
        bytesLeft -= sizeof(foeIdGroup);

        uint32_t strLen = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);
        bytesLeft -= sizeof(uint32_t);

        long copyLength = strLen;
        if (pEndName - pNames < copyLength) {
            copyLength = pEndName - pNames;
        }

        memcpy(pNames, pData, copyLength);
        pData += copyLength;
        bytesLeft -= sizeof(copyLength);

        pNames += copyLength;
        pNames[0] = '\0';
        ++pNames;
    }

    assert(bytesLeft == 0);

    *pDependencyCount = processedCount;
    *pNamesLength = std::min(*pNamesLength, namesLength);

    if (*pNamesLength != 0) {
        --pNames;
        *pNames = '\0';
    }

    return result;
}

foeResultSet getGroupIndexData(std::byte const *pRawData, foeEcsIndexes indexes) {
    std::byte const *pData = pRawData;

    foeIdIndex nextIndex = *(foeIdIndex const *)pData;
    pData += sizeof(foeIdIndex);

    uint32_t freeIndexCount = *(uint32_t const *)pData;
    pData += sizeof(uint32_t);

    foeIdIndex const *pFreeIndexes = (foeIdIndex const *)pData;

    foeEcsImportIndexes(indexes, nextIndex, freeIndexCount, pFreeIndexes);

    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

foeResultSet getGroupResourceIndexData(foeImexImporter importer, foeEcsIndexes indexes) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    return getGroupIndexData(pImporter->pFileData + pImporter->fileHeader.resourceIndexDataOffset,
                             indexes);
}

foeResultSet getGroupEntityIndexData(foeImexImporter importer, foeEcsIndexes indexes) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    return getGroupIndexData(pImporter->pFileData + pImporter->fileHeader.entityIndexDataOffset,
                             indexes);
}

foeResultSet binary_read_EditorName(foeIdIndex indexID,
                                    std::byte const *pRawData,
                                    uint32_t numNames,
                                    uint32_t *pNameLength,
                                    char *pName) {
    std::byte const *pData = pRawData;
    bool found = false;
    uint32_t requestedDataOffset;

    // Find the requested IndexID, if it's here (all the items are in increasing order)
    for (uint32_t i = 0; i < numNames; ++i) {
        foeIdIndex readIndexID = *(foeIdIndex const *)pData;
        pData += sizeof(foeIdIndex);

        if (readIndexID == indexID) {
            found = true;
            requestedDataOffset = *(uint32_t const *)pData;
            break;
        }

        pData += sizeof(uint32_t);
    }
    if (!found)
        return to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_RESOURCE_DATA);

    // We found it, get the name/content
    pData = pRawData + (numNames * (sizeof(foeIdIndex) + sizeof(uint32_t))) + requestedDataOffset;

    uint32_t nameLength = *(uint32_t const *)pData;
    pData += sizeof(uint32_t);

    if (pName == nullptr) {
        *pNameLength = nameLength;
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }

    if (*pNameLength < nameLength) {
        // Partial copy
        memcpy(pName, pData, *pNameLength);
        return to_foeResult(FOE_IMEX_BINARY_INCOMPLETE);
    } else {
        // Copy the full thing
        *pNameLength = nameLength;
        memcpy(pName, pData, nameLength);
        return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    }
}

foeResultSet importStateData(foeImexImporter importer,
                             foeEcsNameMap nameMap,
                             foeSimulation const *pSimulation) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);
    size_t totalDataSize =
        pImporter->fileHeader.fileDataOffset - pImporter->fileHeader.entityDataOffset;
    foeResultSet result = to_foeResult(FOE_IMEX_BINARY_SUCCESS);
    std::byte const *pData;

    auto lock = sharedLockImportFunctionality();
    auto componentFns = getComponentFns();

    if (nameMap != FOE_NULL_HANDLE) {
        // Import all of the Entity EditorNames
        foeIdGroup topLevelImportGroup = foeIdPersistentGroup;
        if (pImporter->groupTranslator != FOE_NULL_HANDLE)
            foeEcsGetTranslatedGroup(pImporter->groupTranslator, foeIdPersistentGroup,
                                     &topLevelImportGroup);

        for (uint32_t i = 0; i < pImporter->fileHeader.numEntityEditorNames; ++i) {
            pData = pImporter->pFileData + pImporter->fileHeader.entityEditorNamesOffset +
                    (i * (sizeof(foeIdIndex) + sizeof(uint32_t)));

            foeIdIndex indexID = *(foeIdIndex const *)pData;
            pData += sizeof(foeIdIndex);

            uint32_t strOffset = *(uint32_t const *)pData;
            pData += sizeof(uint32_t);

            pData = pImporter->pFileData + pImporter->fileHeader.entityEditorNamesOffset +
                    (pImporter->fileHeader.numEntityEditorNames *
                     (sizeof(foeIdIndex) + sizeof(uint32_t))) +
                    strOffset;

            uint32_t strLen = *(uint32_t const *)pData;
            pData += sizeof(uint32_t);

            std::unique_ptr<char[]> buffer(new char[strLen + 1]);
            memcpy(buffer.get(), pData, strLen);
            pData += strLen;
            buffer[strLen] = '\0';

            foeEcsNameMapAdd(nameMap, foeIdCreate(topLevelImportGroup, indexID), buffer.get());
        }
    }

    // Component Binary Key Index
    pData = pImporter->pFileData + pImporter->fileHeader.entityDataOffset;

    uint32_t readSize;
    std::map<uint32_t, std::string_view> keyMap = getKeyMap(pData, &readSize);
    pData += readSize;
    totalDataSize -= readSize;

    // Component Data
    while (totalDataSize > 0) {
        // EntityID
        foeEntityID entity;

        uint32_t readBuffer = 8;
        auto result =
            binary_read_foeEntityID(pData, &readBuffer, pImporter->groupTranslator, &entity);
        pData += readBuffer;
        totalDataSize -= readBuffer;

        // Component Data
        uint32_t componentCount = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);
        totalDataSize -= sizeof(uint32_t);

        for (uint32_t i = 0; i < componentCount; ++i) {
            uint16_t keyIndex = *(uint16_t const *)pData;
            pData += sizeof(uint16_t);
            totalDataSize -= sizeof(uint16_t);

            uint32_t dataSize = *(uint32_t const *)pData;
            pData += sizeof(uint32_t);
            totalDataSize -= sizeof(uint32_t);

            auto indexSearchIt = keyMap.find(keyIndex);
            if (indexSearchIt == keyMap.end()) {
                std::abort();
            }

            auto importSearchIt = componentFns.find(indexSearchIt->second);
            if (importSearchIt == componentFns.end()) {
                std::abort();
            }

            uint32_t processedSize = dataSize;
            result = importSearchIt->second(pData, &processedSize, pImporter->groupTranslator,
                                            entity, pSimulation);
            if (result.value != FOE_SUCCESS)
                return result;

            pData += dataSize;
            totalDataSize -= dataSize;
            assert(processedSize == dataSize);
        }
    }

    return result;
}

foeResultSet getResourceEditorName(foeImexImporter importer,
                                   foeIdIndex resourceIndexID,
                                   uint32_t *pNameLength,
                                   char *pName) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    foeResultSet result = binary_read_EditorName(
        foeIdGetIndex(resourceIndexID),
        pImporter->pFileData + pImporter->fileHeader.resourceEditorNamesOffset,
        pImporter->fileHeader.numResourceEditorNames, pNameLength, pName);

    return result;
}

foeResultSet getResourceCreateInfo(foeImexImporter importer,
                                   foeResourceID resource,
                                   foeResourceCreateInfo *pResourceCreateInfo) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);

    // Search for the resource offset
    std::byte const *pData = pImporter->pFileData + pImporter->fileHeader.resourceIndexOffset;

    size_t entrySize = sizeof(foeIdGroup) + sizeof(foeIdIndex) + sizeof(uint32_t);
    size_t sectionSize =
        pImporter->fileHeader.resourceDataOffset - pImporter->fileHeader.resourceIndexOffset;
    assert(sectionSize % entrySize == 0);
    size_t readSize = 0;

    bool found = false;
    uint32_t desiredDataOffset;
    while (readSize < sectionSize) {
        foeResourceID fileResourceID;

        uint32_t readBuffer = 8;
        auto result = binary_read_foeResourceID(pData, &readBuffer, pImporter->groupTranslator,
                                                &fileResourceID);
        if (result.value != FOE_SUCCESS)
            return result;
        pData += readBuffer;

        if (fileResourceID == resource) {
            found = true;
            desiredDataOffset = *(uint32_t const *)pData;
            pData += sizeof(uint32_t);
            break;
        }

        pData += sizeof(uint32_t);
        readSize += sizeof(foeIdGroup) + sizeof(foeIdIndex) + sizeof(uint32_t);
    }

    if (!found)
        return to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_RESOURCE_DATA);

    // Seek to the desired place
    pData = pImporter->pFileData + pImporter->fileHeader.resourceDataOffset + desiredDataOffset;

    // Read in the data
    uint32_t dataCount = *(uint32_t const *)pData;
    pData += sizeof(uint32_t);

    auto lock = sharedLockImportFunctionality();
    auto resourceFns = getResourceFns();

    assert(dataCount == 1);
    for (uint32_t i = 0; i < dataCount; ++i) {
        uint16_t keyIndex = *(uint16_t const *)pData;
        pData += sizeof(uint16_t);

        uint32_t dataSize = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);

        std::string_view key = pImporter->resourceKeyMap[keyIndex];

        auto searchIt = resourceFns.find(key);
        if (searchIt == resourceFns.end()) {
            std::abort();
        }

        uint32_t processedSize = dataSize;
        foeResourceCreateInfo resourceCI = FOE_NULL_HANDLE;
        foeResultSet result = searchIt->second.importFn(pData, &processedSize,
                                                        pImporter->groupTranslator, &resourceCI);
        if (result.value == FOE_SUCCESS) {
            *pResourceCreateInfo = resourceCI;
            return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
        }
    }

    return to_foeResult(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY);
}

foeResultSet findExternalFile(foeImexImporter importer,
                              char const *pPath,
                              foeManagedMemory *pManagedMemory) {
    foeBinaryImporter *pImporter = importer_from_handle(importer);
    foeResultSet result = to_foeResult(FOE_IMEX_BINARY_ERROR_FAILED_TO_FIND_EXTERNAL_DATA);
    std::byte const *pData = pImporter->pFileData;
    pData += pImporter->fileHeader.fileDataOffset;

    uint32_t const *const cNumFiles = (uint32_t const *)pData;
    pData += sizeof(uint32_t);

    for (uint32_t i = 0; i < *cNumFiles; ++i) {
        uint32_t fileOffset = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);

        uint32_t dataSize = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);

        uint32_t strLen = *(uint32_t const *)pData;
        pData += sizeof(uint32_t);

        std::string_view str{(char const *)pData, strLen};
        pData += strLen;

        if (str == std::string_view{pPath}) {
            result = foeCreateManagedMemorySubset(pImporter->memoryMappedFile, fileOffset, dataSize,
                                                  pManagedMemory);
            break;
        }
    }

    return result;
}

foeImexImporterCalls cImporterCalls{
    .sType = FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS,
    .destroyImporter = destroy,
    .getGroupID = getGroupID,
    .getGroupName = getGroupName,
    .setGroupTranslator = setGroupTranslator,
    .getDependencies = getDependencies,
    .getGroupEntityIndexData = getGroupEntityIndexData,
    .getGroupResourceIndexData = getGroupResourceIndexData,
    .importStateData = importStateData,
    .getResourceEditorName = getResourceEditorName,
    .getResourceCreateInfo = getResourceCreateInfo,
    .findExternalFile = findExternalFile,
};

} // namespace

extern "C" foeResultSet foeCreateBinaryImporter(foeIdGroup group,
                                                char const *pFilePath,
                                                foeImexImporter *pImporter) {
    std::filesystem::path path{pFilePath};

    if (!std::filesystem::exists(path))
        return to_foeResult(FOE_IMEX_BINARY_ERROR_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(path))
        return to_foeResult(FOE_IMEX_BINARY_ERROR_NOT_REGULAR_FILE);

    foeManagedMemory memoryMappedFile = FOE_NULL_HANDLE;
    foeResultSet result = foeCreateMemoryMappedFile(pFilePath, &memoryMappedFile);
    if (result.value != FOE_SUCCESS)
        return result;

    std::byte *pFileData;
    uint32_t fileSize;
    foeManagedMemoryGetData(memoryMappedFile, (void **)&pFileData, &fileSize);

    BinaryFileHeader *pFileHeader = (BinaryFileHeader *)pFileData;
    std::map<uint32_t, std::string_view> resourceKeyMap =
        getKeyMap(pFileData + pFileHeader->resourceBinaryKeyIndexOffset, nullptr);

    // Create the importer
    foeBinaryImporter *pNewImporter = (foeBinaryImporter *)malloc(sizeof(foeBinaryImporter));
    if (pNewImporter == nullptr)
        return to_foeResult(FOE_IMEX_BINARY_ERROR_OUT_OF_MEMORY);

    new (pNewImporter) foeBinaryImporter;
    *pNewImporter = foeBinaryImporter{
        .sType = FOE_IMEX_BINARY_STRUCTURE_TYPE_IMPORTER,
        .pNext = &cImporterCalls,
        .path = path,
        .name = path.stem().string(),
        .group = group,
        .memoryMappedFile = memoryMappedFile,
        .pFileData = pFileData,
        .fileSize = fileSize,
        .fileHeader = *pFileHeader,
        .resourceKeyMap = std::move(resourceKeyMap),
    };

    *pImporter = importer_to_handle(pNewImporter);

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_VERBOSE, "[{}] foeBinaryImporter - Created ({})",
            (void *)pNewImporter, pNewImporter->name.c_str());
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}