// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/binary.h>

#include <foe/binary_result.h>

#include <stdlib.h>

foeResultSet binary_read_foeResourceID(void const *pReadBuffer,
                                       uint32_t *pReadSize,
                                       foeEcsGroupTranslator groupTranslator,
                                       foeResourceID *pResourceID) {
    if (*pReadSize < sizeof(foeIdGroup) + sizeof(foeIdIndex)) {
        return foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
    }

    // Set number of bytes read
    *pReadSize = sizeof(foeIdGroup) + sizeof(foeIdIndex);

    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);

    foeIdGroup *pGroup = (foeIdGroup *)pReadBuffer;
    foeIdIndex *pIndex = (foeIdIndex *)(pGroup + 1);

    if (*pIndex == FOE_INVALID_ID) {
        *pResourceID = FOE_INVALID_ID;
        return result;
    }

    foeIdGroupValue groupValue = *pGroup;
    foeIdGroup group = foeIdPersistentGroup;

    if (groupTranslator == FOE_NULL_HANDLE) {
        if (groupValue != UINT32_MAX) {
            group = foeIdValueToGroup(groupValue);
        }
    } else {
        if (groupValue == UINT32_MAX) {
            result = foeEcsGetTranslatedGroup(groupTranslator, foeIdPersistentGroup, &group);
        } else {
            result =
                foeEcsGetTranslatedGroup(groupTranslator, foeIdValueToGroup(groupValue), &group);
        }
    }

    // Set read data
    *pResourceID = foeIdCreate(group, *pIndex);

    return result;
}

foeResultSet binary_write_foeResourceID(foeResourceID const resourceID,
                                        uint32_t *pWriteSize,
                                        void *pWriteBuffer) {
    if (pWriteBuffer == NULL) {
        *pWriteSize = sizeof(foeIdGroup) + sizeof(foeIdIndex);
        return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    }

    if (*pWriteSize < sizeof(foeIdGroup) + sizeof(foeIdIndex)) {
        // The given buffer is too small, return an error with the required minimum size
        *pWriteSize = sizeof(foeIdGroup) + sizeof(foeIdIndex);
        return foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
    }

    foeIdGroup *pGroup = (foeIdGroup *)pWriteBuffer;
    foeIdIndex *pIndex = (foeIdIndex *)(pGroup + 1);

    if (foeIdGetGroup(resourceID) == foeIdPersistentGroup) {
        // Persistent group, set as max possible value, all 1's
        *pGroup = UINT32_MAX;
    } else {
        *pGroup = foeIdGroupToValue(resourceID);
    }
    *pIndex = foeIdGetIndex(resourceID);

    // Return bytes written and success result
    *pWriteSize = sizeof(foeIdGroup) + sizeof(foeIdIndex);
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

foeResultSet binary_read_foeEntityID(void const *pReadBuffer,
                                     uint32_t *pReadSize,
                                     foeEcsGroupTranslator groupTranslator,
                                     foeEntityID *pEntityID) {
    return binary_read_foeResourceID(pReadBuffer, pReadSize, groupTranslator, pEntityID);
}

foeResultSet binary_write_foeEntityID(foeEntityID const entityID,
                                      uint32_t *pWriteSize,
                                      void *pWriteBuffer) {
    return binary_write_foeResourceID(entityID, pWriteSize, pWriteBuffer);
}
