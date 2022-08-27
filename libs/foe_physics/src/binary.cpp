// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/binary.h>

#include <foe/binary_result.h>
#include <foe/ecs/binary.h>
#include <foe/physics/component/rigid_body.h>
#include <foe/physics/resource/collision_shape_create_info.hpp>

#include <string.h>

extern "C" foeResultSet binary_read_foeRigidBody(void const *pReadBuffer,
                                                 uint32_t *pReadSize,
                                                 foeEcsGroupTranslator groupTranslator,
                                                 foeRigidBody *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeRigidBody newData;
    memset(&newData, 0, sizeof(foeRigidBody));

    // float - mass
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_RIGID_BODY_READ_FAILED;
    }
    memcpy(&newData.mass, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

    // foeResourceID - collisionShape
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.collisionShape);
        if (result.value != FOE_SUCCESS)
            goto FOE_RIGID_BODY_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

FOE_RIGID_BODY_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

extern "C" foeResultSet binary_write_foeRigidBody(foeRigidBody const *pData,
                                                  uint32_t *pWriteSize,
                                                  void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // float - mass
    writeSize += sizeof(float);

    // foeResourceID - collisionShape
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->collisionShape, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

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

    // float - mass
    memcpy(writePtr, &pData->mass, sizeof(float));
    writePtr += sizeof(float);

    // foeResourceID - collisionShape
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->collisionShape, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foeRigidBody() {
    return "17fa35c4691df1d32a24ce23c0c8488620c117116330114b8c46b74b";
}

extern "C" foeResultSet binary_read_foeCollisionShapeCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, foeCollisionShapeCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeCollisionShapeCreateInfo newData;
    memset(&newData, 0, sizeof(foeCollisionShapeCreateInfo));

    // glm::vec3 - boxSize
    if (bufferSizeLeft < sizeof(glm::vec3)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_COLLISION_SHAPE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.boxSize, readPtr, sizeof(glm::vec3));
    readPtr += sizeof(glm::vec3);
    bufferSizeLeft -= sizeof(glm::vec3);

FOE_COLLISION_SHAPE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

extern "C" foeResultSet binary_write_foeCollisionShapeCreateInfo(
    foeCollisionShapeCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // glm::vec3 - boxSize
    writeSize += sizeof(glm::vec3);

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

    // glm::vec3 - boxSize
    memcpy(writePtr, &pData->boxSize, sizeof(glm::vec3));
    writePtr += sizeof(glm::vec3);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foeCollisionShapeCreateInfo() {
    return "27bcbaed8d6dee23131e8bcbd7fcc8b471e53778d17ecc64564dbb10";
}
