// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/binary.h>

#include <foe/binary_result.h>
#include <foe/position/component/3d.hpp>

#include <string.h>

extern "C" foeResultSet binary_read_foePosition3d(void const *pReadBuffer,
                                                  uint32_t *pReadSize,
                                                  foePosition3d *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foePosition3d newData;
    memset(&newData, 0, sizeof(foePosition3d));

    // glm::vec3 - position
    if (bufferSizeLeft < sizeof(glm::vec3)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_POSITION3D_READ_FAILED;
    }
    memcpy(&newData.position, readPtr, sizeof(glm::vec3));
    readPtr += sizeof(glm::vec3);
    bufferSizeLeft -= sizeof(glm::vec3);

    // glm::quat - orientation
    if (bufferSizeLeft < sizeof(glm::quat)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_POSITION3D_READ_FAILED;
    }
    memcpy(&newData.orientation, readPtr, sizeof(glm::quat));
    readPtr += sizeof(glm::quat);
    bufferSizeLeft -= sizeof(glm::quat);

FOE_POSITION3D_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

extern "C" foeResultSet binary_write_foePosition3d(foePosition3d const *pData,
                                                   uint32_t *pWriteSize,
                                                   void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // glm::vec3 - position
    writeSize += sizeof(glm::vec3);

    // glm::quat - orientation
    writeSize += sizeof(glm::quat);

    if (pWriteBuffer == NULL) {
        // If there is no buffer to write to, just return the required buffer size
        *pWriteSize = writeSize;
        return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    }

    if (*pWriteSize < writeSize) {
        // The given buffer is too small, return an error with the required minimum size
        *pWriteSize = writeSize;
        return foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
    }

    uint8_t *writePtr = (uint8_t *)pWriteBuffer;

    // glm::vec3 - position
    memcpy(writePtr, &pData->position, sizeof(glm::vec3));
    writePtr += sizeof(glm::vec3);

    // glm::quat - orientation
    memcpy(writePtr, &pData->orientation, sizeof(glm::quat));
    writePtr += sizeof(glm::quat);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foePosition3d() {
    return "b776c7b9f0685f2aa94c78927dd564bbafcfe8bd72fb7dc368e43732";
}
