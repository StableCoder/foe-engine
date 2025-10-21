// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/binary.h>

#include <foe/binary_result.h>
#include <foe/ecs/binary.h>
#include <foe/external/vk_struct_compare.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/vk/binary.h>
#include <foe/graphics/vk/compare.h>
#include <foe/graphics/vk/vk_binary.h>

#include <stdlib.h>
#include <string.h>

foeResultSet binary_read_foeImageCreateInfo(void const *pReadBuffer,
                                            uint32_t *pReadSize,
                                            foeImageCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeImageCreateInfo newData;
    memset(&newData, 0, sizeof(foeImageCreateInfo));

    // char const * - pFile[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_IMAGE_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_IMAGE_CREATE_INFO_READ_FAILED;
            }
            newData.pFile = (char *)malloc(strLen + 1);
            if (newData.pFile == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_IMAGE_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pFile, readPtr, strLen);
            ((char *)newData.pFile)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

FOE_IMAGE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeImageCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeImageCreateInfo(foeImageCreateInfo const *pData,
                                             uint32_t *pWriteSize,
                                             void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // char const * - pFile[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pFile) {
        writeSize += strlen(pData->pFile);
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

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeImageCreateInfo() {
    return "12dea8746da10638c5762a891dc6f47c259b28b62e919aae2428f7c2";
}

foeResultSet binary_read_foeMeshFileCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeMeshFileCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeMeshFileCreateInfo newData;
    memset(&newData, 0, sizeof(foeMeshFileCreateInfo));

    // char const * - pFile[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
            }
            newData.pFile = (char *)malloc(strLen + 1);
            if (newData.pFile == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pFile, readPtr, strLen);
            ((char *)newData.pFile)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // char const * - pMesh[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
            }
            newData.pMesh = (char *)malloc(strLen + 1);
            if (newData.pMesh == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pMesh, readPtr, strLen);
            ((char *)newData.pMesh)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // unsigned int - postProcessFlags
    if (bufferSizeLeft < sizeof(unsigned int)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_MESH_FILE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.postProcessFlags, readPtr, sizeof(unsigned int));
    readPtr += sizeof(unsigned int);
    bufferSizeLeft -= sizeof(unsigned int);

FOE_MESH_FILE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeMeshFileCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // char const * - pFile[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pFile) {
        writeSize += strlen(pData->pFile);
    }

    // char const * - pMesh[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pMesh) {
        writeSize += strlen(pData->pMesh);
    }

    // unsigned int - postProcessFlags
    writeSize += sizeof(unsigned int);

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

    // char const * - pMesh[null-terminated]
    {
        uint32_t strLen = (pData->pMesh) ? strlen(pData->pMesh) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pMesh, strLen);
        writePtr += strLen;
    }

    // unsigned int - postProcessFlags
    memcpy(writePtr, &pData->postProcessFlags, sizeof(unsigned int));
    writePtr += sizeof(unsigned int);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeMeshFileCreateInfo() {
    return "6ec3760910b44db3ddb54b4a05575c6b7a33a73f4b42e72e5d7fe0ba";
}

foeResultSet binary_read_foeMeshCubeCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeMeshCubeCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeMeshCubeCreateInfo newData;
    memset(&newData, 0, sizeof(foeMeshCubeCreateInfo));

FOE_MESH_CUBE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

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

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeMeshCubeCreateInfo() {
    return "b2329f178dc10327d28e015c1e417bd30d98b16afc3e97685c7aa6fa";
}

foeResultSet binary_read_foeMeshIcosphereCreateInfo(void const *pReadBuffer,
                                                    uint32_t *pReadSize,
                                                    foeMeshIcosphereCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeMeshIcosphereCreateInfo newData;
    memset(&newData, 0, sizeof(foeMeshIcosphereCreateInfo));

    // int - recursion
    if (bufferSizeLeft < sizeof(int)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_MESH_ICOSPHERE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.recursion, readPtr, sizeof(int));
    readPtr += sizeof(int);
    bufferSizeLeft -= sizeof(int);

FOE_MESH_ICOSPHERE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pData,
                                                     uint32_t *pWriteSize,
                                                     void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // int - recursion
    writeSize += sizeof(int);

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

    // int - recursion
    memcpy(writePtr, &pData->recursion, sizeof(int));
    writePtr += sizeof(int);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeMeshIcosphereCreateInfo() {
    return "25f6d8853af9e9c9235cf9b143ee3870d67b4cb4faf0aa88a7f2e471";
}

foeResultSet binary_read_foeMaterialCreateInfo(void const *pReadBuffer,
                                               uint32_t *pReadSize,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeMaterialCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeMaterialCreateInfo newData;
    memset(&newData, 0, sizeof(foeMaterialCreateInfo));

    // foeResourceID - fragmentShader
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.fragmentShader);
        if (result.value != FOE_SUCCESS)
            goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - image
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator, &newData.image);
        if (result.value != FOE_SUCCESS)
            goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
    {
        if (readPtr - (uint8_t const *)pReadBuffer < sizeof(uint8_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
        }
        uint8_t memberWritten;
        memcpy(&memberWritten, readPtr, sizeof(uint8_t));
        readPtr += sizeof(uint8_t);
        bufferSizeLeft -= sizeof(uint8_t);

        if (memberWritten) {
            newData.pRasterizationSCI = calloc(1, sizeof(VkPipelineRasterizationStateCreateInfo));
            if (newData.pRasterizationSCI == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            }

            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkPipelineRasterizationStateCreateInfo(
                readPtr, &subReadSize,
                (VkPipelineRasterizationStateCreateInfo *)newData.pRasterizationSCI);
            if (result.value != FOE_SUCCESS)
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
    {
        if (readPtr - (uint8_t const *)pReadBuffer < sizeof(uint8_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
        }
        uint8_t memberWritten;
        memcpy(&memberWritten, readPtr, sizeof(uint8_t));
        readPtr += sizeof(uint8_t);
        bufferSizeLeft -= sizeof(uint8_t);

        if (memberWritten) {
            newData.pDepthStencilSCI = calloc(1, sizeof(VkPipelineDepthStencilStateCreateInfo));
            if (newData.pDepthStencilSCI == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            }

            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkPipelineDepthStencilStateCreateInfo(
                readPtr, &subReadSize,
                (VkPipelineDepthStencilStateCreateInfo *)newData.pDepthStencilSCI);
            if (result.value != FOE_SUCCESS)
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
    {
        if (readPtr - (uint8_t const *)pReadBuffer < sizeof(uint8_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
        }
        uint8_t memberWritten;
        memcpy(&memberWritten, readPtr, sizeof(uint8_t));
        readPtr += sizeof(uint8_t);
        bufferSizeLeft -= sizeof(uint8_t);

        if (memberWritten) {
            newData.pColourBlendSCI = calloc(1, sizeof(VkPipelineColorBlendStateCreateInfo));
            if (newData.pColourBlendSCI == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            }

            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkPipelineColorBlendStateCreateInfo(
                readPtr, &subReadSize,
                (VkPipelineColorBlendStateCreateInfo *)newData.pColourBlendSCI);
            if (result.value != FOE_SUCCESS)
                goto FOE_MATERIAL_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

FOE_MATERIAL_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeMaterialCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeMaterialCreateInfo(foeMaterialCreateInfo const *pData,
                                                uint32_t *pWriteSize,
                                                void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // foeResourceID - fragmentShader
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->fragmentShader, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - image
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->image, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
    writeSize += sizeof(uint8_t);
    if (pData->pRasterizationSCI) {
        uint32_t subWriteSize;
        binary_write_VkPipelineRasterizationStateCreateInfo(pData->pRasterizationSCI, &subWriteSize,
                                                            NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
    writeSize += sizeof(uint8_t);
    if (pData->pDepthStencilSCI) {
        uint32_t subWriteSize;
        binary_write_VkPipelineDepthStencilStateCreateInfo(pData->pDepthStencilSCI, &subWriteSize,
                                                           NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
    writeSize += sizeof(uint8_t);
    if (pData->pColourBlendSCI) {
        uint32_t subWriteSize;
        binary_write_VkPipelineColorBlendStateCreateInfo(pData->pColourBlendSCI, &subWriteSize,
                                                         NULL);
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

    // foeResourceID - fragmentShader
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->fragmentShader, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - image
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_foeResourceID(pData->image, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
    {
        uint8_t writeMember = (pData->pRasterizationSCI) ? 1 : 0;
        memcpy(writePtr, &writeMember, sizeof(uint8_t));
        writePtr += sizeof(uint8_t);
    }
    if (pData->pRasterizationSCI) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineRasterizationStateCreateInfo(
            pData->pRasterizationSCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
    {
        uint8_t writeMember = (pData->pDepthStencilSCI) ? 1 : 0;
        memcpy(writePtr, &writeMember, sizeof(uint8_t));
        writePtr += sizeof(uint8_t);
    }
    if (pData->pDepthStencilSCI) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineDepthStencilStateCreateInfo(
            pData->pDepthStencilSCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
    {
        uint8_t writeMember = (pData->pColourBlendSCI) ? 1 : 0;
        memcpy(writePtr, &writeMember, sizeof(uint8_t));
        writePtr += sizeof(uint8_t);
    }
    if (pData->pColourBlendSCI) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineColorBlendStateCreateInfo(
            pData->pColourBlendSCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeMaterialCreateInfo() {
    return "842ce122772d271179070f502fa401a62068cd6e9af35167b2c68c8e";
}

foeResultSet binary_read_foeShaderCreateInfo(void const *pReadBuffer,
                                             uint32_t *pReadSize,
                                             foeShaderCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeShaderCreateInfo newData;
    memset(&newData, 0, sizeof(foeShaderCreateInfo));

    // char const * - pFile[null-terminated]
    {
        if (bufferSizeLeft < sizeof(uint32_t)) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            goto FOE_SHADER_CREATE_INFO_READ_FAILED;
        }
        uint32_t strLen;
        memcpy(&strLen, readPtr, sizeof(uint32_t));
        readPtr += sizeof(uint32_t);
        bufferSizeLeft -= sizeof(uint32_t);

        if (strLen > 0) {
            if (bufferSizeLeft < strLen) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                goto FOE_SHADER_CREATE_INFO_READ_FAILED;
            }
            newData.pFile = (char *)malloc(strLen + 1);
            if (newData.pFile == NULL) {
                result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
                goto FOE_SHADER_CREATE_INFO_READ_FAILED;
            }
            memcpy((char *)newData.pFile, readPtr, strLen);
            ((char *)newData.pFile)[strLen] = (char)0;
            readPtr += strLen;
            bufferSizeLeft -= strLen;
        }
    }

    // foeGfxVkShaderCreateInfo - gfxCreateInfo
    {
        uint32_t subReadSize = bufferSizeLeft;
        result =
            binary_read_foeGfxVkShaderCreateInfo(readPtr, &subReadSize, &newData.gfxCreateInfo);
        if (result.value != FOE_SUCCESS)
            goto FOE_SHADER_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

FOE_SHADER_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeShaderCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeShaderCreateInfo(foeShaderCreateInfo const *pData,
                                              uint32_t *pWriteSize,
                                              void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // char const * - pFile[null-terminated]
    writeSize += sizeof(uint32_t);
    if (pData->pFile) {
        writeSize += strlen(pData->pFile);
    }

    // foeGfxVkShaderCreateInfo - gfxCreateInfo
    {
        uint32_t subWriteSize;
        binary_write_foeGfxVkShaderCreateInfo(&pData->gfxCreateInfo, &subWriteSize, NULL);
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

    // char const * - pFile[null-terminated]
    {
        uint32_t strLen = (pData->pFile) ? strlen(pData->pFile) : 0;
        memcpy(writePtr, &strLen, sizeof(uint32_t));
        writePtr += sizeof(uint32_t);

        memcpy(writePtr, pData->pFile, strLen);
        writePtr += strLen;
    }

    // foeGfxVkShaderCreateInfo - gfxCreateInfo
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeGfxVkShaderCreateInfo(&pData->gfxCreateInfo, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeShaderCreateInfo() {
    return "97303c8ec8dc6a450f6f5e3910e6644b40bc55cba310aefe87436f23";
}

foeResultSet binary_read_foeVertexDescriptorCreateInfo(void const *pReadBuffer,
                                                       uint32_t *pReadSize,
                                                       foeEcsGroupTranslator groupTranslator,
                                                       foeVertexDescriptorCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeVertexDescriptorCreateInfo newData;
    memset(&newData, 0, sizeof(foeVertexDescriptorCreateInfo));

    // uint32_t - inputBindingCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.inputBindingCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - inputAttributeCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.inputAttributeCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // foeResourceID - vertexShader
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.vertexShader);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - tessellationControlShader
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.tessellationControlShader);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - tessellationEvaluationShader
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.tessellationEvaluationShader);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // foeResourceID - geometryShader
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_foeResourceID(readPtr, &subReadSize, groupTranslator,
                                           &newData.geometryShader);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkPipelineVertexInputStateCreateInfo(readPtr, &subReadSize,
                                                                  &newData.vertexInputSCI);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
    if (newData.inputBindingCount > 0) {
        newData.pInputBindings = (VkVertexInputBindingDescription *)calloc(
            newData.inputBindingCount, sizeof(VkVertexInputBindingDescription));
        if (newData.pInputBindings == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.inputBindingCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkVertexInputBindingDescription(
                readPtr, &subReadSize,
                (VkVertexInputBindingDescription *)newData.pInputBindings + i);
            if (result.value != FOE_SUCCESS)
                goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
    if (newData.inputAttributeCount > 0) {
        newData.pInputAttributes = (VkVertexInputAttributeDescription *)calloc(
            newData.inputAttributeCount, sizeof(VkVertexInputAttributeDescription));
        if (newData.pInputAttributes == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.inputAttributeCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkVertexInputAttributeDescription(
                readPtr, &subReadSize,
                (VkVertexInputAttributeDescription *)newData.pInputAttributes + i);
            if (result.value != FOE_SUCCESS)
                goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkPipelineInputAssemblyStateCreateInfo(readPtr, &subReadSize,
                                                                    &newData.inputAssemblySCI);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkPipelineTessellationStateCreateInfo - tessellationSCI
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkPipelineTessellationStateCreateInfo(readPtr, &subReadSize,
                                                                   &newData.tessellationSCI);
        if (result.value != FOE_SUCCESS)
            goto FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

FOE_VERTEX_DESCRIPTOR_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeVertexDescriptorCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pData,
                                                        uint32_t *pWriteSize,
                                                        void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - inputBindingCount
    writeSize += sizeof(uint32_t);

    // uint32_t - inputAttributeCount
    writeSize += sizeof(uint32_t);

    // foeResourceID - vertexShader
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->vertexShader, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - tessellationControlShader
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->tessellationControlShader, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - tessellationEvaluationShader
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->tessellationEvaluationShader, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // foeResourceID - geometryShader
    {
        uint32_t subWriteSize;
        binary_write_foeResourceID(pData->geometryShader, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
    {
        uint32_t subWriteSize;
        binary_write_VkPipelineVertexInputStateCreateInfo(&pData->vertexInputSCI, &subWriteSize,
                                                          NULL);
        writeSize += subWriteSize;
    }

    // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
    for (size_t i = 0; i < pData->inputBindingCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkVertexInputBindingDescription(pData->pInputBindings + i, &subWriteSize,
                                                     NULL);
        writeSize += subWriteSize;
    }

    // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
    for (size_t i = 0; i < pData->inputAttributeCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkVertexInputAttributeDescription(pData->pInputAttributes + i, &subWriteSize,
                                                       NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
    {
        uint32_t subWriteSize;
        binary_write_VkPipelineInputAssemblyStateCreateInfo(&pData->inputAssemblySCI, &subWriteSize,
                                                            NULL);
        writeSize += subWriteSize;
    }

    // VkPipelineTessellationStateCreateInfo - tessellationSCI
    {
        uint32_t subWriteSize;
        binary_write_VkPipelineTessellationStateCreateInfo(&pData->tessellationSCI, &subWriteSize,
                                                           NULL);
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

    // uint32_t - inputBindingCount
    memcpy(writePtr, &pData->inputBindingCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - inputAttributeCount
    memcpy(writePtr, &pData->inputAttributeCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // foeResourceID - vertexShader
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->vertexShader, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - tessellationControlShader
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->tessellationControlShader, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - tessellationEvaluationShader
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_foeResourceID(pData->tessellationEvaluationShader,
                                                         &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // foeResourceID - geometryShader
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_foeResourceID(pData->geometryShader, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineVertexInputStateCreateInfo(
            &pData->vertexInputSCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
    for (size_t i = 0; i < pData->inputBindingCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkVertexInputBindingDescription(
            pData->pInputBindings + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
    for (size_t i = 0; i < pData->inputAttributeCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkVertexInputAttributeDescription(
            pData->pInputAttributes + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineInputAssemblyStateCreateInfo(
            &pData->inputAssemblySCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPipelineTessellationStateCreateInfo - tessellationSCI
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineTessellationStateCreateInfo(
            &pData->tessellationSCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeVertexDescriptorCreateInfo() {
    return "4157bdf182fce3c08e54734881201dfe0588d98240b4bdb3eb711994";
}
