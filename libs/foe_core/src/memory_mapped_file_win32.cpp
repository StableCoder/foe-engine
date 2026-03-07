// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/memory_mapped_file.h>

#include "result.h"

#include <windows.h>

struct MemoryMappedFile {
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD fileSize;
    HANDLE mappedFile = INVALID_HANDLE_VALUE;
    void *pData;
};

void cleanup_MemoryMappedFile(void *pData, uint32_t dataSize, void *pMetadata) {
    MemoryMappedFile *pMemoryMappedFile = (MemoryMappedFile *)pMetadata;
    BOOL success;

    if (pMemoryMappedFile->pData) {
        success = UnmapViewOfFile(pMemoryMappedFile->pData);
        if (!success) {
            // ??
        }
    }

    if (pMemoryMappedFile->mappedFile != INVALID_HANDLE_VALUE) {
        success = CloseHandle(pMemoryMappedFile->mappedFile);
        if (!success) {
            // ??
        }
    }

    if (pMemoryMappedFile->file != INVALID_HANDLE_VALUE) {
        success = CloseHandle(pMemoryMappedFile->file);
        if (!success) {
            // ??
        }
    }
}

foeResultSet foeCreateMemoryMappedFile(char const *pFilePath, foeManagedMemory *pManagedMemory) {
    MemoryMappedFile mappedFileData = {
        .file = INVALID_HANDLE_VALUE,
        .mappedFile = INVALID_HANDLE_VALUE,
    };
    foeResultSet result;

    mappedFileData.file =
        CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (mappedFileData.file == INVALID_HANDLE_VALUE)
        return to_foeResult(FOE_ERROR_FAILED_TO_OPEN_FILE);

    mappedFileData.fileSize = GetFileSize(mappedFileData.file, NULL);
    if (mappedFileData.fileSize == 0) {
        result = to_foeResult(FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE);
        goto CREATE_FAILED;
    }

    mappedFileData.mappedFile = CreateFileMapping(mappedFileData.file, NULL, PAGE_READONLY, 0,
                                                  mappedFileData.fileSize, NULL);
    if (mappedFileData.mappedFile == NULL) {
        result = to_foeResult(FOE_ERROR_FAILED_TO_MAP_FILE);
        goto CREATE_FAILED;
    }

    mappedFileData.pData =
        MapViewOfFile(mappedFileData.mappedFile, FILE_MAP_READ, 0, 0, mappedFileData.fileSize);
    if (mappedFileData.pData == NULL) {
        result = to_foeResult(FOE_ERROR_FAILED_TO_MAP_FILE);
        goto CREATE_FAILED;
    }

    result = foeCreateManagedMemory(mappedFileData.pData, mappedFileData.fileSize,
                                    cleanup_MemoryMappedFile, &mappedFileData,
                                    sizeof(mappedFileData), pManagedMemory);

CREATE_FAILED:
    if (result.value != FOE_SUCCESS)
        cleanup_MemoryMappedFile(NULL, 0, &mappedFileData);

    return result;
}