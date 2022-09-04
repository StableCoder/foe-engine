// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape.h"

#include <foe/physics/binary.h>
#include <foe/physics/type_defs.h>

#include "result.h"

#include <stdlib.h>
#include <string.h>

foeResultSet export_foeCollisionShapeCreateInfo(foeResourceCreateInfo createInfo,
                                                foeImexBinarySet *pBinarySet,
                                                foeImexBinaryFiles *pFiles) {
    if (createInfo == FOE_NULL_HANDLE) {
        return to_foeResult(FOE_PHYSICS_BINARY_ERROR_NO_CREATE_INFO_PROVIDED);
    }

    foeResourceCreateInfoType ciType = foeResourceCreateInfoGetType(createInfo);
    foeResultSet result = to_foeResult(FOE_PHYSICS_BINARY_DATA_NOT_EXPORTED);

    foeImexBinarySet set;
    memset(&set, 0, sizeof(foeImexBinarySet));
    foeImexBinaryFiles files;
    memset(&files, 0, sizeof(foeImexBinaryFiles));

    if (ciType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO) {
        result = binary_write_foeCollisionShapeCreateInfo(foeResourceCreateInfoGetData(createInfo),
                                                          &set.dataSize, NULL);
        if (result.value != FOE_SUCCESS)
            goto EXPORT_FAILED;

        set.pData = malloc(set.dataSize);
        if (set.pData == NULL) {
            result = to_foeResult(FOE_PHYSICS_BINARY_ERROR_OUT_OF_MEMORY);
            goto EXPORT_FAILED;
        }

        result = binary_write_foeCollisionShapeCreateInfo(foeResourceCreateInfoGetData(createInfo),
                                                          &set.dataSize, set.pData);
        set.pKey = binary_key_foeCollisionShapeCreateInfo();
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
