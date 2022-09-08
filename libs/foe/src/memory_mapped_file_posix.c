// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/memory_mapped_file.h>

#include "result.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct MemoryMappedFile {
    int fileDescriptor;
    void *pData;
    long dataSize;
} MemoryMappedFile;

static void cleanup_MemoryMappedFile(void *pMemoryMappedData) {
    MemoryMappedFile *pMemoryMappedFile = pMemoryMappedData;
    int posixRetVal;

    if (pMemoryMappedFile->pData && pMemoryMappedFile->pData != (void *)-1) {
        posixRetVal = munmap(pMemoryMappedFile->pData, pMemoryMappedFile->dataSize);
        if (posixRetVal == -1) {
            // READ ERRNO
        }
    }

    if (pMemoryMappedFile->fileDescriptor > 0) {
        posixRetVal = close(pMemoryMappedFile->fileDescriptor);
        if (posixRetVal == -1) {
            // READ ERRNO
        }
    }
}

foeResultSet foeCreateMemoryMappedFile(char const *pFilePath, foeManagedMemory *pManagedMemory) {
    MemoryMappedFile mappedFileData = {};
    foeResultSet result;

    mappedFileData.fileDescriptor = open(pFilePath, O_RDONLY);
    if (mappedFileData.fileDescriptor == -1)
        return to_foeResult(FOE_ERROR_FAILED_TO_OPEN_FILE);

    struct stat statData = {};
    int posixRetVal = stat(pFilePath, &statData);
    if (posixRetVal == -1) {
        result = to_foeResult(FOE_ERROR_FAILED_TO_OPEN_FILE);
        goto CREATE_FAILED;
    }
    mappedFileData.dataSize = statData.st_size;

    if (statData.st_size == 0) {
        result = to_foeResult(FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE);
        goto CREATE_FAILED;
    }

    mappedFileData.pData = (uint8_t *)mmap(NULL, mappedFileData.dataSize, PROT_READ, MAP_PRIVATE,
                                           mappedFileData.fileDescriptor, 0);
    if (mappedFileData.pData == (void *)-1) {
        result = to_foeResult(FOE_ERROR_FAILED_TO_MAP_FILE);
        goto CREATE_FAILED;
    }

    result = foeCreateManagedMemory(mappedFileData.pData, mappedFileData.dataSize,
                                    cleanup_MemoryMappedFile, &mappedFileData,
                                    sizeof(mappedFileData), pManagedMemory);

CREATE_FAILED:
    if (result.value != FOE_SUCCESS)
        cleanup_MemoryMappedFile(&mappedFileData);

    return result;
}
