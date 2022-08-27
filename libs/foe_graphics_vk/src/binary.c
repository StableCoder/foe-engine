// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/binary.h>

#include <foe/binary_result.h>
#include <foe/graphics/vk/cleanup.h>
#include <foe/graphics/vk/shader.h>
#include <foe/graphics/vk/vk_binary.h>
#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <string.h>

foeResultSet binary_read_foeGfxVkShaderCreateInfo(void const *pReadBuffer,
                                                  uint32_t *pReadSize,
                                                  foeGfxVkShaderCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    foeGfxVkShaderCreateInfo newData;
    memset(&newData, 0, sizeof(foeGfxVkShaderCreateInfo));

    // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
    if (bufferSizeLeft < sizeof(foeBuiltinDescriptorSetLayoutFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto FOE_GFX_VK_SHADER_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.builtinSetLayouts, readPtr, sizeof(foeBuiltinDescriptorSetLayoutFlags));
    readPtr += sizeof(foeBuiltinDescriptorSetLayoutFlags);
    bufferSizeLeft -= sizeof(foeBuiltinDescriptorSetLayoutFlags);

    // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkDescriptorSetLayoutCreateInfo(readPtr, &subReadSize,
                                                             &newData.descriptorSetLayoutCI);
        if (result.value != FOE_SUCCESS)
            goto FOE_GFX_VK_SHADER_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkPushConstantRange - pushConstantRange
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkPushConstantRange(readPtr, &subReadSize, &newData.pushConstantRange);
        if (result.value != FOE_SUCCESS)
            goto FOE_GFX_VK_SHADER_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

FOE_GFX_VK_SHADER_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_foeGfxVkShaderCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_foeGfxVkShaderCreateInfo(foeGfxVkShaderCreateInfo const *pData,
                                                   uint32_t *pWriteSize,
                                                   void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
    writeSize += sizeof(foeBuiltinDescriptorSetLayoutFlags);

    // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
    {
        uint32_t subWriteSize;
        binary_write_VkDescriptorSetLayoutCreateInfo(&pData->descriptorSetLayoutCI, &subWriteSize,
                                                     NULL);
        writeSize += subWriteSize;
    }

    // VkPushConstantRange - pushConstantRange
    {
        uint32_t subWriteSize;
        binary_write_VkPushConstantRange(&pData->pushConstantRange, &subWriteSize, NULL);
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

    // foeBuiltinDescriptorSetLayoutFlags - builtinSetLayouts
    memcpy(writePtr, &pData->builtinSetLayouts, sizeof(foeBuiltinDescriptorSetLayoutFlags));
    writePtr += sizeof(foeBuiltinDescriptorSetLayoutFlags);

    // VkDescriptorSetLayoutCreateInfo - descriptorSetLayoutCI
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkDescriptorSetLayoutCreateInfo(
            &pData->descriptorSetLayoutCI, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkPushConstantRange - pushConstantRange
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result =
            binary_write_VkPushConstantRange(&pData->pushConstantRange, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_foeGfxVkShaderCreateInfo() {
    return "3f0124502b54e2a8c0aad546391f1caad352fa43b8110f25ef9a1851";
}
