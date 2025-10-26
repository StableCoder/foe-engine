// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "binary.h"

#include <foe/binary_result.h>
#include <foe/ecs/binary.h>

#include "armature_create_info.h"
#include "armature_state.h"
#include "cleanup.h"
#include "render_state.h"

#include <stdlib.h>
#include <string.h>

extern "C" foeResultSet binary_read_AnimationImportInfo(void const *pReadBuffer,
                                                        uint32_t *pReadSize,
                                                        AnimationImportInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    AnimationImportInfo newData;
    memset(&newData, 0, sizeof(AnimationImportInfo));

    // char const * - pFile[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto ANIMATION_IMPORT_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto ANIMATION_IMPORT_INFO_READ_FAILED;
            }
            newData.pFile = (char *)malloc(strLen + 1);
            if (newData.pFile == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto ANIMATION_IMPORT_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pFile, readPtr, strLen);
            ((char *)newData.pFile)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // char const * - pName[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto ANIMATION_IMPORT_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto ANIMATION_IMPORT_INFO_READ_FAILED;
            }
            newData.pName = (char *)malloc(strLen + 1);
            if (newData.pName == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto ANIMATION_IMPORT_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pName, readPtr, strLen);
            ((char *)newData.pName)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

ANIMATION_IMPORT_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_AnimationImportInfo(&newData);
    }

    return result;
}

extern "C" foeResultSet binary_write_AnimationImportInfo(AnimationImportInfo const *pData,
                                                         uint32_t *pWriteSize,
                                                         void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // char const * - pFile[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pFile) {
        writeSize += strlen(pData->pFile);
    }

    // char const * - pName[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pName) {
        writeSize += strlen(pData->pName);
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

    // char const * - pFile[null-terminated]
    {
        uint32_t strLen = (pData->pFile) ? strlen(pData->pFile) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pFile, strLen);
        writePtr += strLen;
    }

    // char const * - pName[null-terminated]
    {
        uint32_t strLen = (pData->pName) ? strlen(pData->pName) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pName, strLen);
        writePtr += strLen;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_AnimationImportInfo() {
    return "4c31564399e9e3d3fb5005808891de1880b1495abc3ed934a87ad626";
}

extern "C" foeResultSet binary_read_foeArmatureCreateInfo(void const *pReadBuffer,
                                                          uint32_t *pReadSize,
                                                          foeArmatureCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeArmatureCreateInfo newData;
    memset(&newData, 0, sizeof(foeArmatureCreateInfo));

    // uint32_t - animationCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.animationCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // char const * - pFile[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
            }
            newData.pFile = (char *)malloc(strLen + 1);
            if (newData.pFile == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pFile, readPtr, strLen);
            ((char *)newData.pFile)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // char const * - pRootArmatureNode[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
            }
            newData.pRootArmatureNode = (char *)malloc(strLen + 1);
            if (newData.pRootArmatureNode == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pRootArmatureNode, readPtr, strLen);
            ((char *)newData.pRootArmatureNode)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // AnimationImportInfo* - pAnimations[animationCount]
    if (newData.animationCount > 0) {
        newData.pAnimations =
            (AnimationImportInfo *)calloc(newData.animationCount, sizeof(AnimationImportInfo));
        if (newData.pAnimations == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.animationCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_AnimationImportInfo(
                readPtr, &subReadSize, (AnimationImportInfo *)newData.pAnimations + i);
            if (result.value != FOE_SUCCESS)
                goto FOE_ARMATURE_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

FOE_ARMATURE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeArmatureCreateInfo(&newData);
    }

    return result;
}

extern "C" foeResultSet binary_write_foeArmatureCreateInfo(foeArmatureCreateInfo const *pData,
                                                           uint32_t *pWriteSize,
                                                           void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - animationCount
    writeSize += sizeof(uint32_t);

    // char const * - pFile[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pFile) {
        writeSize += strlen(pData->pFile);
    }

    // char const * - pRootArmatureNode[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pRootArmatureNode) {
        writeSize += strlen(pData->pRootArmatureNode);
    }

    // AnimationImportInfo* - pAnimations[animationCount]
    for (size_t i = 0; i < pData->animationCount; ++i) {
        uint32_t subWriteSize;
        binary_write_AnimationImportInfo(pData->pAnimations + i, &subWriteSize, NULL);
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

    // uint32_t - animationCount
    memcpy(writePtr, &pData->animationCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // char const * - pFile[null-terminated]
    {
        uint32_t strLen = (pData->pFile) ? strlen(pData->pFile) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pFile, strLen);
        writePtr += strLen;
    }

    // char const * - pRootArmatureNode[null-terminated]
    {
        uint32_t strLen = (pData->pRootArmatureNode) ? strlen(pData->pRootArmatureNode) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pRootArmatureNode, strLen);
        writePtr += strLen;
    }

    // AnimationImportInfo* - pAnimations[animationCount]
    for (size_t i = 0; i < pData->animationCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_AnimationImportInfo(pData->pAnimations + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foeArmatureCreateInfo() {
    return "a54fceee6c7996a1b0919af41f8a372689db56758df4b57c1bef6e35";
}

extern "C" foeResultSet binary_read_foeArmatureState(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeEcsGroupTranslator groupTranslator,
                                                     foeArmatureState *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeArmatureState newData;
    memset(&newData, 0, sizeof(foeArmatureState));

    // foeResourceID - armatureID
    {
        uint32_t subReadSize = bufferSizeLeft;
        result =
            binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator, &newData.armatureID);
        if (result.value != FOE_SUCCESS)
            goto FOE_ARMATURE_STATE_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // uint32_t - animationID
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_ARMATURE_STATE_READ_FAILED;
    }
    memcpy(&newData.animationID, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // float - time
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_ARMATURE_STATE_READ_FAILED;
    }
    memcpy(&newData.time, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

FOE_ARMATURE_STATE_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

extern "C" foeResultSet binary_write_foeArmatureState(foeArmatureState const *pData,
                                                      uint32_t *pWriteSize,
                                                      void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // foeResourceID - armatureID
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->armatureID, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // uint32_t - animationID
    writeSize += sizeof(uint32_t);

    // float - time
    writeSize += sizeof(float);

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

    // foeResourceID - armatureID
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->armatureID, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // uint32_t - animationID
    memcpy(writePtr, &pData->animationID, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // float - time
    memcpy(writePtr, &pData->time, sizeof(float));
    writePtr += sizeof(float);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foeArmatureState() {
    return "8385dd2b487c215e7e0378bdc04448abe137aa16be3c59cc0ac7e7f0";
}

extern "C" foeResultSet binary_read_foeRenderState(void const *pReadBuffer,
                                                   uint32_t *pReadSize,
                                                   foeEcsGroupTranslator groupTranslator,
                                                   foeRenderState *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeRenderState newData;
    memset(&newData, 0, sizeof(foeRenderState));

    // foeResourceID - vertexDescriptor
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.vertexDescriptor);
        if (result.value != FOE_SUCCESS)
            goto FOE_RENDER_STATE_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - bonedVertexDescriptor
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.bonedVertexDescriptor);
        if (result.value != FOE_SUCCESS)
            goto FOE_RENDER_STATE_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - material
    {
        uint32_t subReadSize = bufferSizeLeft;
        result =
            binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator, &newData.material);
        if (result.value != FOE_SUCCESS)
            goto FOE_RENDER_STATE_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - mesh
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator, &newData.mesh);
        if (result.value != FOE_SUCCESS)
            goto FOE_RENDER_STATE_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

FOE_RENDER_STATE_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

extern "C" foeResultSet binary_write_foeRenderState(foeRenderState const *pData,
                                                    uint32_t *pWriteSize,
                                                    void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // foeResourceID - vertexDescriptor
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->vertexDescriptor, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - bonedVertexDescriptor
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->bonedVertexDescriptor, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - material
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->material, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - mesh
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->mesh, &subWriteSize, NULL);
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

    // foeResourceID - vertexDescriptor
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->vertexDescriptor, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - bonedVertexDescriptor
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->bonedVertexDescriptor, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - material
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_foeResourceID(pData->material, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - mesh
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_foeResourceID(pData->mesh, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

extern "C" char const *binary_key_foeRenderState() {
    return "83f56db7ebc3f808d9bf1303609833106caac72abcb4d2de2445afa1";
}
