// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/type_defs.h>

#include "result.h"

#include <stdlib.h>
#include <string.h>

foeResultSet export_foeShaderCreateInfo(foeResourceCreateInfo createInfo,
                                        foeImexBinarySet *pBinarySet,
                                        foeImexBinaryFiles *pFiles) {
    if (createInfo == FOE_NULL_HANDLE) {
        return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_ERROR_NO_CREATE_INFO_PROVIDED);
    }

    foeResourceCreateInfoType ciType = foeResourceCreateInfoGetType(createInfo);
    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_DATA_NOT_EXPORTED);

    foeImexBinarySet set;
    memset(&set, 0, sizeof(foeImexBinarySet));
    foeImexBinaryFiles files;
    memset(&files, 0, sizeof(foeImexBinaryFiles));

    if (ciType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO) {
        foeShaderCreateInfo const *pData = foeResourceCreateInfoGetData(createInfo);

        result = binary_write_foeShaderCreateInfo(pData, &set.dataSize, NULL);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;

        set.pData = malloc(set.dataSize);
        if (set.pData == NULL) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_ERROR_OUT_OF_MEMORY);
            goto EXPORT_FAILED;
        }

        result = binary_write_foeShaderCreateInfo(pData, &set.dataSize, set.pData);
        set.pKey = binary_key_foeShaderCreateInfo();

        files.fileCount = 1;
        files.ppFiles = malloc(sizeof(char const *) * files.fileCount);
        files.ppFiles[0] = pData->pFile;
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