// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.h"

#include "../armature_create_info.h"
#include "../binary.h"
#include "../type_defs.h"
#include "result.h"

#include <stdlib.h>
#include <string.h>

foeResultSet export_foeArmatureCreateInfo(foeResourceCreateInfo createInfo,
                                          foeImexBinarySet *pBinarySet,
                                          foeImexBinaryFiles *pFiles) {
    if (createInfo == FOE_NULL_HANDLE) {
        return to_foeResult(FOE_SKUNKWORKS_BINARY_ERROR_NO_CREATE_INFO_PROVIDED);
    }

    foeResourceCreateInfoType ciType = foeResourceCreateInfoGetType(createInfo);
    foeResultSet result = to_foeResult(FOE_SKUNKWORKS_BINARY_DATA_NOT_EXPORTED);

    foeImexBinarySet set;
    memset(&set, 0, sizeof(foeImexBinarySet));
    foeImexBinaryFiles files;
    memset(&files, 0, sizeof(foeImexBinaryFiles));

    // foeArmature
    if (ciType == FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_CREATE_INFO) {
        foeArmatureCreateInfo *pResourceCI =
            (foeArmatureCreateInfo *)foeResourceCreateInfoGetData(createInfo);

        result = binary_write_foeArmatureCreateInfo(pResourceCI, &set.dataSize, NULL);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;

        set.pData = malloc(set.dataSize);
        if (set.pData == NULL) {
            result = to_foeResult(FOE_SKUNKWORKS_BINARY_ERROR_OUT_OF_MEMORY);
            goto EXPORT_FAILED;
        }

        result = binary_write_foeArmatureCreateInfo(pResourceCI, &set.dataSize, set.pData);
        set.pKey = binary_key_foeArmatureCreateInfo();

        files.fileCount = 1 + pResourceCI->animationCount;
        files.ppFiles = malloc(sizeof(char const *) * files.fileCount);

        files.ppFiles[0] = pResourceCI->pFile;
        for (size_t i = 0; i < pResourceCI->animationCount; ++i) {
            files.ppFiles[1 + i] = pResourceCI->pAnimations[i].pFile;
        }
    }

EXPORT_FAILED:
    if (result.value != FOE_SUCCESS) {
        if (set.pData)
            free(set.pData);
        if (files.ppFiles)
            free(files.ppFiles);
    } else if (result.value == FOE_SUCCESS) {
        *pBinarySet = set;
        *pFiles = files;
    }

    return result;
}
