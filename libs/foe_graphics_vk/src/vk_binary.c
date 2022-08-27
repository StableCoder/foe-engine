// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/vk_binary.h>

#include <foe/binary_result.h>
#include <vk_struct_cleanup.h>
#include <vk_struct_compare.h>

#include <stdlib.h>
#include <string.h>

foeResultSet binary_read_VkPushConstantRange(void const *pReadBuffer,
                                             uint32_t *pReadSize,
                                             VkPushConstantRange *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPushConstantRange newData;
    memset(&newData, 0, sizeof(VkPushConstantRange));

    // VkShaderStageFlags - stageFlags
    if (bufferSizeLeft < sizeof(VkShaderStageFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PUSH_CONSTANT_RANGE_READ_FAILED;
    }
    memcpy(&newData.stageFlags, readPtr, sizeof(VkShaderStageFlags));
    readPtr += sizeof(VkShaderStageFlags);
    bufferSizeLeft -= sizeof(VkShaderStageFlags);

    // uint32_t - offset
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PUSH_CONSTANT_RANGE_READ_FAILED;
    }
    memcpy(&newData.offset, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - size
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PUSH_CONSTANT_RANGE_READ_FAILED;
    }
    memcpy(&newData.size, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

VK_PUSH_CONSTANT_RANGE_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_VkPushConstantRange(VkPushConstantRange const *pData,
                                              uint32_t *pWriteSize,
                                              void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkShaderStageFlags - stageFlags
    writeSize += sizeof(VkShaderStageFlags);

    // uint32_t - offset
    writeSize += sizeof(uint32_t);

    // uint32_t - size
    writeSize += sizeof(uint32_t);

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

    // VkShaderStageFlags - stageFlags
    memcpy(writePtr, &pData->stageFlags, sizeof(VkShaderStageFlags));
    writePtr += sizeof(VkShaderStageFlags);

    // uint32_t - offset
    memcpy(writePtr, &pData->offset, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - size
    memcpy(writePtr, &pData->size, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPushConstantRange() {
    return "5793eaa59507898a001990e6476c23fd8d87005ce8e7a1cbf3d624d6";
}

foeResultSet binary_read_VkDescriptorSetLayoutBinding(void const *pReadBuffer,
                                                      uint32_t *pReadSize,
                                                      VkDescriptorSetLayoutBinding *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkDescriptorSetLayoutBinding newData;
    memset(&newData, 0, sizeof(VkDescriptorSetLayoutBinding));

    // uint32_t - descriptorCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_BINDING_READ_FAILED;
    }
    memcpy(&newData.descriptorCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - binding
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_BINDING_READ_FAILED;
    }
    memcpy(&newData.binding, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkDescriptorType - descriptorType
    if (bufferSizeLeft < sizeof(VkDescriptorType)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_BINDING_READ_FAILED;
    }
    memcpy(&newData.descriptorType, readPtr, sizeof(VkDescriptorType));
    readPtr += sizeof(VkDescriptorType);
    bufferSizeLeft -= sizeof(VkDescriptorType);

    // VkShaderStageFlags - stageFlags
    if (bufferSizeLeft < sizeof(VkShaderStageFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_BINDING_READ_FAILED;
    }
    memcpy(&newData.stageFlags, readPtr, sizeof(VkShaderStageFlags));
    readPtr += sizeof(VkShaderStageFlags);
    bufferSizeLeft -= sizeof(VkShaderStageFlags);

VK_DESCRIPTOR_SET_LAYOUT_BINDING_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkDescriptorSetLayoutBinding(&newData);
    }

    return result;
}

foeResultSet binary_write_VkDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding const *pData,
                                                       uint32_t *pWriteSize,
                                                       void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - descriptorCount
    writeSize += sizeof(uint32_t);

    // uint32_t - binding
    writeSize += sizeof(uint32_t);

    // VkDescriptorType - descriptorType
    writeSize += sizeof(VkDescriptorType);

    // VkShaderStageFlags - stageFlags
    writeSize += sizeof(VkShaderStageFlags);

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

    // uint32_t - descriptorCount
    memcpy(writePtr, &pData->descriptorCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - binding
    memcpy(writePtr, &pData->binding, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkDescriptorType - descriptorType
    memcpy(writePtr, &pData->descriptorType, sizeof(VkDescriptorType));
    writePtr += sizeof(VkDescriptorType);

    // VkShaderStageFlags - stageFlags
    memcpy(writePtr, &pData->stageFlags, sizeof(VkShaderStageFlags));
    writePtr += sizeof(VkShaderStageFlags);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkDescriptorSetLayoutBinding() {
    return "55bf721020eb226a4203d07c1e7fe69dc5fe49bc73cda8943368b227";
}

foeResultSet binary_read_VkDescriptorSetLayoutCreateInfo(void const *pReadBuffer,
                                                         uint32_t *pReadSize,
                                                         VkDescriptorSetLayoutCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkDescriptorSetLayoutCreateInfo newData;
    memset(&newData, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    // uint32_t - bindingCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.bindingCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // VkDescriptorSetLayoutCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkDescriptorSetLayoutCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkDescriptorSetLayoutCreateFlags));
    readPtr += sizeof(VkDescriptorSetLayoutCreateFlags);
    bufferSizeLeft -= sizeof(VkDescriptorSetLayoutCreateFlags);

    // VkDescriptorSetLayoutBinding* - pBindings[bindingCount]
    if (newData.bindingCount > 0) {
        newData.pBindings = (VkDescriptorSetLayoutBinding *)calloc(
            newData.bindingCount, sizeof(VkDescriptorSetLayoutBinding));
        if (newData.pBindings == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.bindingCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkDescriptorSetLayoutBinding(
                readPtr, &subReadSize, (VkDescriptorSetLayoutBinding *)newData.pBindings + i);
            if (result.value != FOE_SUCCESS)
                goto VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkDescriptorSetLayoutCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkDescriptorSetLayoutCreateInfo(
    VkDescriptorSetLayoutCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - bindingCount
    writeSize += sizeof(uint32_t);

    // VkDescriptorSetLayoutCreateFlags - flags
    writeSize += sizeof(VkDescriptorSetLayoutCreateFlags);

    // VkDescriptorSetLayoutBinding* - pBindings[bindingCount]
    for (size_t i = 0; i < pData->bindingCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkDescriptorSetLayoutBinding(pData->pBindings + i, &subWriteSize, NULL);
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

    // uint32_t - bindingCount
    memcpy(writePtr, &pData->bindingCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkDescriptorSetLayoutCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkDescriptorSetLayoutCreateFlags));
    writePtr += sizeof(VkDescriptorSetLayoutCreateFlags);

    // VkDescriptorSetLayoutBinding* - pBindings[bindingCount]
    for (size_t i = 0; i < pData->bindingCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkDescriptorSetLayoutBinding(pData->pBindings + i,
                                                                        &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkDescriptorSetLayoutCreateInfo() {
    return "18fd7643828876d371d4ba56a0043475603318513e10ae25bf89b45b";
}

foeResultSet binary_read_VkStencilOpState(void const *pReadBuffer,
                                          uint32_t *pReadSize,
                                          VkStencilOpState *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkStencilOpState newData;
    memset(&newData, 0, sizeof(VkStencilOpState));

    // VkStencilOp - failOp
    if (bufferSizeLeft < sizeof(VkStencilOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.failOp, readPtr, sizeof(VkStencilOp));
    readPtr += sizeof(VkStencilOp);
    bufferSizeLeft -= sizeof(VkStencilOp);

    // VkStencilOp - passOp
    if (bufferSizeLeft < sizeof(VkStencilOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.passOp, readPtr, sizeof(VkStencilOp));
    readPtr += sizeof(VkStencilOp);
    bufferSizeLeft -= sizeof(VkStencilOp);

    // VkStencilOp - depthFailOp
    if (bufferSizeLeft < sizeof(VkStencilOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.depthFailOp, readPtr, sizeof(VkStencilOp));
    readPtr += sizeof(VkStencilOp);
    bufferSizeLeft -= sizeof(VkStencilOp);

    // VkCompareOp - compareOp
    if (bufferSizeLeft < sizeof(VkCompareOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.compareOp, readPtr, sizeof(VkCompareOp));
    readPtr += sizeof(VkCompareOp);
    bufferSizeLeft -= sizeof(VkCompareOp);

    // uint32_t - compareMask
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.compareMask, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - writeMask
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.writeMask, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - reference
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_STENCIL_OP_STATE_READ_FAILED;
    }
    memcpy(&newData.reference, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

VK_STENCIL_OP_STATE_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_VkStencilOpState(VkStencilOpState const *pData,
                                           uint32_t *pWriteSize,
                                           void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkStencilOp - failOp
    writeSize += sizeof(VkStencilOp);

    // VkStencilOp - passOp
    writeSize += sizeof(VkStencilOp);

    // VkStencilOp - depthFailOp
    writeSize += sizeof(VkStencilOp);

    // VkCompareOp - compareOp
    writeSize += sizeof(VkCompareOp);

    // uint32_t - compareMask
    writeSize += sizeof(uint32_t);

    // uint32_t - writeMask
    writeSize += sizeof(uint32_t);

    // uint32_t - reference
    writeSize += sizeof(uint32_t);

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

    // VkStencilOp - failOp
    memcpy(writePtr, &pData->failOp, sizeof(VkStencilOp));
    writePtr += sizeof(VkStencilOp);

    // VkStencilOp - passOp
    memcpy(writePtr, &pData->passOp, sizeof(VkStencilOp));
    writePtr += sizeof(VkStencilOp);

    // VkStencilOp - depthFailOp
    memcpy(writePtr, &pData->depthFailOp, sizeof(VkStencilOp));
    writePtr += sizeof(VkStencilOp);

    // VkCompareOp - compareOp
    memcpy(writePtr, &pData->compareOp, sizeof(VkCompareOp));
    writePtr += sizeof(VkCompareOp);

    // uint32_t - compareMask
    memcpy(writePtr, &pData->compareMask, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - writeMask
    memcpy(writePtr, &pData->writeMask, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - reference
    memcpy(writePtr, &pData->reference, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkStencilOpState() {
    return "a6e086adaf2fd6b95f90af007fb8c4cab7faaa0e5e5d00debcc7b8a9";
}

foeResultSet binary_read_VkPipelineRasterizationStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineRasterizationStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineRasterizationStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    // VkPipelineRasterizationStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineRasterizationStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineRasterizationStateCreateFlags));
    readPtr += sizeof(VkPipelineRasterizationStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineRasterizationStateCreateFlags);

    // VkBool32 - depthClampEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthClampEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkBool32 - rasterizerDiscardEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.rasterizerDiscardEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkPolygonMode - polygonMode
    if (bufferSizeLeft < sizeof(VkPolygonMode)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.polygonMode, readPtr, sizeof(VkPolygonMode));
    readPtr += sizeof(VkPolygonMode);
    bufferSizeLeft -= sizeof(VkPolygonMode);

    // VkCullModeFlags - cullMode
    if (bufferSizeLeft < sizeof(VkCullModeFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.cullMode, readPtr, sizeof(VkCullModeFlags));
    readPtr += sizeof(VkCullModeFlags);
    bufferSizeLeft -= sizeof(VkCullModeFlags);

    // VkFrontFace - frontFace
    if (bufferSizeLeft < sizeof(VkFrontFace)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.frontFace, readPtr, sizeof(VkFrontFace));
    readPtr += sizeof(VkFrontFace);
    bufferSizeLeft -= sizeof(VkFrontFace);

    // VkBool32 - depthBiasEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthBiasEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // float - depthBiasConstantFactor
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthBiasConstantFactor, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

    // float - depthBiasClamp
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthBiasClamp, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

    // float - depthBiasSlopeFactor
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthBiasSlopeFactor, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

    // float - lineWidth
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.lineWidth, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

VK_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineRasterizationStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineRasterizationStateCreateInfo(
    VkPipelineRasterizationStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkPipelineRasterizationStateCreateFlags - flags
    writeSize += sizeof(VkPipelineRasterizationStateCreateFlags);

    // VkBool32 - depthClampEnable
    writeSize += sizeof(VkBool32);

    // VkBool32 - rasterizerDiscardEnable
    writeSize += sizeof(VkBool32);

    // VkPolygonMode - polygonMode
    writeSize += sizeof(VkPolygonMode);

    // VkCullModeFlags - cullMode
    writeSize += sizeof(VkCullModeFlags);

    // VkFrontFace - frontFace
    writeSize += sizeof(VkFrontFace);

    // VkBool32 - depthBiasEnable
    writeSize += sizeof(VkBool32);

    // float - depthBiasConstantFactor
    writeSize += sizeof(float);

    // float - depthBiasClamp
    writeSize += sizeof(float);

    // float - depthBiasSlopeFactor
    writeSize += sizeof(float);

    // float - lineWidth
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

    // VkPipelineRasterizationStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineRasterizationStateCreateFlags));
    writePtr += sizeof(VkPipelineRasterizationStateCreateFlags);

    // VkBool32 - depthClampEnable
    memcpy(writePtr, &pData->depthClampEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkBool32 - rasterizerDiscardEnable
    memcpy(writePtr, &pData->rasterizerDiscardEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkPolygonMode - polygonMode
    memcpy(writePtr, &pData->polygonMode, sizeof(VkPolygonMode));
    writePtr += sizeof(VkPolygonMode);

    // VkCullModeFlags - cullMode
    memcpy(writePtr, &pData->cullMode, sizeof(VkCullModeFlags));
    writePtr += sizeof(VkCullModeFlags);

    // VkFrontFace - frontFace
    memcpy(writePtr, &pData->frontFace, sizeof(VkFrontFace));
    writePtr += sizeof(VkFrontFace);

    // VkBool32 - depthBiasEnable
    memcpy(writePtr, &pData->depthBiasEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // float - depthBiasConstantFactor
    memcpy(writePtr, &pData->depthBiasConstantFactor, sizeof(float));
    writePtr += sizeof(float);

    // float - depthBiasClamp
    memcpy(writePtr, &pData->depthBiasClamp, sizeof(float));
    writePtr += sizeof(float);

    // float - depthBiasSlopeFactor
    memcpy(writePtr, &pData->depthBiasSlopeFactor, sizeof(float));
    writePtr += sizeof(float);

    // float - lineWidth
    memcpy(writePtr, &pData->lineWidth, sizeof(float));
    writePtr += sizeof(float);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineRasterizationStateCreateInfo() {
    return "700c515f6b015aca2c895cb0d34ce536c97ddd75fb5f1936b70867b5";
}

foeResultSet binary_read_VkPipelineDepthStencilStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineDepthStencilStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineDepthStencilStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // VkPipelineDepthStencilStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineDepthStencilStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineDepthStencilStateCreateFlags));
    readPtr += sizeof(VkPipelineDepthStencilStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineDepthStencilStateCreateFlags);

    // VkBool32 - depthTestEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthTestEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkBool32 - depthWriteEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthWriteEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkCompareOp - depthCompareOp
    if (bufferSizeLeft < sizeof(VkCompareOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthCompareOp, readPtr, sizeof(VkCompareOp));
    readPtr += sizeof(VkCompareOp);
    bufferSizeLeft -= sizeof(VkCompareOp);

    // VkBool32 - depthBoundsTestEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.depthBoundsTestEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkBool32 - stencilTestEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.stencilTestEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkStencilOpState - front
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkStencilOpState(readPtr, &subReadSize, &newData.front);
        if (result.value != FOE_SUCCESS)
            goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // VkStencilOpState - back
    {
        uint32_t subReadSize = bufferSizeLeft;
        result = binary_read_VkStencilOpState(readPtr, &subReadSize, &newData.back);
        if (result.value != FOE_SUCCESS)
            goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
        readPtr += subReadSize;
        bufferSizeLeft -= subReadSize;
    }

    // float - minDepthBounds
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.minDepthBounds, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

    // float - maxDepthBounds
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.maxDepthBounds, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineDepthStencilStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineDepthStencilStateCreateInfo(
    VkPipelineDepthStencilStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkPipelineDepthStencilStateCreateFlags - flags
    writeSize += sizeof(VkPipelineDepthStencilStateCreateFlags);

    // VkBool32 - depthTestEnable
    writeSize += sizeof(VkBool32);

    // VkBool32 - depthWriteEnable
    writeSize += sizeof(VkBool32);

    // VkCompareOp - depthCompareOp
    writeSize += sizeof(VkCompareOp);

    // VkBool32 - depthBoundsTestEnable
    writeSize += sizeof(VkBool32);

    // VkBool32 - stencilTestEnable
    writeSize += sizeof(VkBool32);

    // VkStencilOpState - front
    {
        uint32_t subWriteSize;
        binary_write_VkStencilOpState(&pData->front, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // VkStencilOpState - back
    {
        uint32_t subWriteSize;
        binary_write_VkStencilOpState(&pData->back, &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // float - minDepthBounds
    writeSize += sizeof(float);

    // float - maxDepthBounds
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

    // VkPipelineDepthStencilStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineDepthStencilStateCreateFlags));
    writePtr += sizeof(VkPipelineDepthStencilStateCreateFlags);

    // VkBool32 - depthTestEnable
    memcpy(writePtr, &pData->depthTestEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkBool32 - depthWriteEnable
    memcpy(writePtr, &pData->depthWriteEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkCompareOp - depthCompareOp
    memcpy(writePtr, &pData->depthCompareOp, sizeof(VkCompareOp));
    writePtr += sizeof(VkCompareOp);

    // VkBool32 - depthBoundsTestEnable
    memcpy(writePtr, &pData->depthBoundsTestEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkBool32 - stencilTestEnable
    memcpy(writePtr, &pData->stencilTestEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkStencilOpState - front
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkStencilOpState(&pData->front, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkStencilOpState - back
    {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkStencilOpState(&pData->back, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // float - minDepthBounds
    memcpy(writePtr, &pData->minDepthBounds, sizeof(float));
    writePtr += sizeof(float);

    // float - maxDepthBounds
    memcpy(writePtr, &pData->maxDepthBounds, sizeof(float));
    writePtr += sizeof(float);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineDepthStencilStateCreateInfo() {
    return "dc9077222bf39f8d5b4b4c6cc1c7cd9372a1eb25192154c7609dc330";
}

foeResultSet binary_read_VkPipelineColorBlendAttachmentState(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineColorBlendAttachmentState *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineColorBlendAttachmentState newData;
    memset(&newData, 0, sizeof(VkPipelineColorBlendAttachmentState));

    // VkBool32 - blendEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.blendEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkBlendFactor - srcColorBlendFactor
    if (bufferSizeLeft < sizeof(VkBlendFactor)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.srcColorBlendFactor, readPtr, sizeof(VkBlendFactor));
    readPtr += sizeof(VkBlendFactor);
    bufferSizeLeft -= sizeof(VkBlendFactor);

    // VkBlendFactor - dstColorBlendFactor
    if (bufferSizeLeft < sizeof(VkBlendFactor)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.dstColorBlendFactor, readPtr, sizeof(VkBlendFactor));
    readPtr += sizeof(VkBlendFactor);
    bufferSizeLeft -= sizeof(VkBlendFactor);

    // VkBlendOp - colorBlendOp
    if (bufferSizeLeft < sizeof(VkBlendOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.colorBlendOp, readPtr, sizeof(VkBlendOp));
    readPtr += sizeof(VkBlendOp);
    bufferSizeLeft -= sizeof(VkBlendOp);

    // VkBlendFactor - srcAlphaBlendFactor
    if (bufferSizeLeft < sizeof(VkBlendFactor)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.srcAlphaBlendFactor, readPtr, sizeof(VkBlendFactor));
    readPtr += sizeof(VkBlendFactor);
    bufferSizeLeft -= sizeof(VkBlendFactor);

    // VkBlendFactor - dstAlphaBlendFactor
    if (bufferSizeLeft < sizeof(VkBlendFactor)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.dstAlphaBlendFactor, readPtr, sizeof(VkBlendFactor));
    readPtr += sizeof(VkBlendFactor);
    bufferSizeLeft -= sizeof(VkBlendFactor);

    // VkBlendOp - alphaBlendOp
    if (bufferSizeLeft < sizeof(VkBlendOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.alphaBlendOp, readPtr, sizeof(VkBlendOp));
    readPtr += sizeof(VkBlendOp);
    bufferSizeLeft -= sizeof(VkBlendOp);

    // VkColorComponentFlags - colorWriteMask
    if (bufferSizeLeft < sizeof(VkColorComponentFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED;
    }
    memcpy(&newData.colorWriteMask, readPtr, sizeof(VkColorComponentFlags));
    readPtr += sizeof(VkColorComponentFlags);
    bufferSizeLeft -= sizeof(VkColorComponentFlags);

VK_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_VkPipelineColorBlendAttachmentState(
    VkPipelineColorBlendAttachmentState const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkBool32 - blendEnable
    writeSize += sizeof(VkBool32);

    // VkBlendFactor - srcColorBlendFactor
    writeSize += sizeof(VkBlendFactor);

    // VkBlendFactor - dstColorBlendFactor
    writeSize += sizeof(VkBlendFactor);

    // VkBlendOp - colorBlendOp
    writeSize += sizeof(VkBlendOp);

    // VkBlendFactor - srcAlphaBlendFactor
    writeSize += sizeof(VkBlendFactor);

    // VkBlendFactor - dstAlphaBlendFactor
    writeSize += sizeof(VkBlendFactor);

    // VkBlendOp - alphaBlendOp
    writeSize += sizeof(VkBlendOp);

    // VkColorComponentFlags - colorWriteMask
    writeSize += sizeof(VkColorComponentFlags);

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

    // VkBool32 - blendEnable
    memcpy(writePtr, &pData->blendEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkBlendFactor - srcColorBlendFactor
    memcpy(writePtr, &pData->srcColorBlendFactor, sizeof(VkBlendFactor));
    writePtr += sizeof(VkBlendFactor);

    // VkBlendFactor - dstColorBlendFactor
    memcpy(writePtr, &pData->dstColorBlendFactor, sizeof(VkBlendFactor));
    writePtr += sizeof(VkBlendFactor);

    // VkBlendOp - colorBlendOp
    memcpy(writePtr, &pData->colorBlendOp, sizeof(VkBlendOp));
    writePtr += sizeof(VkBlendOp);

    // VkBlendFactor - srcAlphaBlendFactor
    memcpy(writePtr, &pData->srcAlphaBlendFactor, sizeof(VkBlendFactor));
    writePtr += sizeof(VkBlendFactor);

    // VkBlendFactor - dstAlphaBlendFactor
    memcpy(writePtr, &pData->dstAlphaBlendFactor, sizeof(VkBlendFactor));
    writePtr += sizeof(VkBlendFactor);

    // VkBlendOp - alphaBlendOp
    memcpy(writePtr, &pData->alphaBlendOp, sizeof(VkBlendOp));
    writePtr += sizeof(VkBlendOp);

    // VkColorComponentFlags - colorWriteMask
    memcpy(writePtr, &pData->colorWriteMask, sizeof(VkColorComponentFlags));
    writePtr += sizeof(VkColorComponentFlags);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineColorBlendAttachmentState() {
    return "407f019b6a1e3f575584e20e60c920f6b62e86c43c4829dc868b3d08";
}

foeResultSet binary_read_VkPipelineColorBlendStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineColorBlendStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineColorBlendStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

    // uint32_t - attachmentCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.attachmentCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    // VkPipelineColorBlendStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineColorBlendStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineColorBlendStateCreateFlags));
    readPtr += sizeof(VkPipelineColorBlendStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineColorBlendStateCreateFlags);

    // VkBool32 - logicOpEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.logicOpEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

    // VkLogicOp - logicOp
    if (bufferSizeLeft < sizeof(VkLogicOp)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.logicOp, readPtr, sizeof(VkLogicOp));
    readPtr += sizeof(VkLogicOp);
    bufferSizeLeft -= sizeof(VkLogicOp);

    // VkPipelineColorBlendAttachmentState* - pAttachments[attachmentCount]
    if (newData.attachmentCount > 0) {
        newData.pAttachments = (VkPipelineColorBlendAttachmentState *)calloc(
            newData.attachmentCount, sizeof(VkPipelineColorBlendAttachmentState));
        if (newData.pAttachments == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.attachmentCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkPipelineColorBlendAttachmentState(
                readPtr, &subReadSize,
                (VkPipelineColorBlendAttachmentState *)newData.pAttachments + i);
            if (result.value != FOE_SUCCESS)
                goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // float - blendConstants
    if (bufferSizeLeft < sizeof(float)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.blendConstants, readPtr, sizeof(float));
    readPtr += sizeof(float);
    bufferSizeLeft -= sizeof(float);

VK_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineColorBlendStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineColorBlendStateCreateInfo(
    VkPipelineColorBlendStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - attachmentCount
    writeSize += sizeof(uint32_t);

    // VkPipelineColorBlendStateCreateFlags - flags
    writeSize += sizeof(VkPipelineColorBlendStateCreateFlags);

    // VkBool32 - logicOpEnable
    writeSize += sizeof(VkBool32);

    // VkLogicOp - logicOp
    writeSize += sizeof(VkLogicOp);

    // VkPipelineColorBlendAttachmentState* - pAttachments[attachmentCount]
    for (size_t i = 0; i < pData->attachmentCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkPipelineColorBlendAttachmentState(pData->pAttachments + i, &subWriteSize,
                                                         NULL);
        writeSize += subWriteSize;
    }

    // float - blendConstants
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

    // uint32_t - attachmentCount
    memcpy(writePtr, &pData->attachmentCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkPipelineColorBlendStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineColorBlendStateCreateFlags));
    writePtr += sizeof(VkPipelineColorBlendStateCreateFlags);

    // VkBool32 - logicOpEnable
    memcpy(writePtr, &pData->logicOpEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // VkLogicOp - logicOp
    memcpy(writePtr, &pData->logicOp, sizeof(VkLogicOp));
    writePtr += sizeof(VkLogicOp);

    // VkPipelineColorBlendAttachmentState* - pAttachments[attachmentCount]
    for (size_t i = 0; i < pData->attachmentCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkPipelineColorBlendAttachmentState(
            pData->pAttachments + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // float - blendConstants
    memcpy(writePtr, &pData->blendConstants, sizeof(float));
    writePtr += sizeof(float);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineColorBlendStateCreateInfo() {
    return "444f33725d0d5b4f109294d0c3d305afc7b30ed8baab264fb1c215be";
}

foeResultSet binary_read_VkVertexInputBindingDescription(void const *pReadBuffer,
                                                         uint32_t *pReadSize,
                                                         VkVertexInputBindingDescription *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkVertexInputBindingDescription newData;
    memset(&newData, 0, sizeof(VkVertexInputBindingDescription));

    // uint32_t - binding
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_BINDING_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.binding, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - stride
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_BINDING_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.stride, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkVertexInputRate - inputRate
    if (bufferSizeLeft < sizeof(VkVertexInputRate)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_BINDING_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.inputRate, readPtr, sizeof(VkVertexInputRate));
    readPtr += sizeof(VkVertexInputRate);
    bufferSizeLeft -= sizeof(VkVertexInputRate);

VK_VERTEX_INPUT_BINDING_DESCRIPTION_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_VkVertexInputBindingDescription(
    VkVertexInputBindingDescription const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - binding
    writeSize += sizeof(uint32_t);

    // uint32_t - stride
    writeSize += sizeof(uint32_t);

    // VkVertexInputRate - inputRate
    writeSize += sizeof(VkVertexInputRate);

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

    // uint32_t - binding
    memcpy(writePtr, &pData->binding, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - stride
    memcpy(writePtr, &pData->stride, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkVertexInputRate - inputRate
    memcpy(writePtr, &pData->inputRate, sizeof(VkVertexInputRate));
    writePtr += sizeof(VkVertexInputRate);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkVertexInputBindingDescription() {
    return "1d67ec20bab4be7e66270b9c6f106110f439a6490266c20911fb9602";
}

foeResultSet binary_read_VkVertexInputAttributeDescription(
    void const *pReadBuffer, uint32_t *pReadSize, VkVertexInputAttributeDescription *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkVertexInputAttributeDescription newData;
    memset(&newData, 0, sizeof(VkVertexInputAttributeDescription));

    // uint32_t - location
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.location, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - binding
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.binding, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkFormat - format
    if (bufferSizeLeft < sizeof(VkFormat)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.format, readPtr, sizeof(VkFormat));
    readPtr += sizeof(VkFormat);
    bufferSizeLeft -= sizeof(VkFormat);

    // uint32_t - offset
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_READ_FAILED;
    }
    memcpy(&newData.offset, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

VK_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    }

    return result;
}

foeResultSet binary_write_VkVertexInputAttributeDescription(
    VkVertexInputAttributeDescription const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - location
    writeSize += sizeof(uint32_t);

    // uint32_t - binding
    writeSize += sizeof(uint32_t);

    // VkFormat - format
    writeSize += sizeof(VkFormat);

    // uint32_t - offset
    writeSize += sizeof(uint32_t);

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

    // uint32_t - location
    memcpy(writePtr, &pData->location, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - binding
    memcpy(writePtr, &pData->binding, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkFormat - format
    memcpy(writePtr, &pData->format, sizeof(VkFormat));
    writePtr += sizeof(VkFormat);

    // uint32_t - offset
    memcpy(writePtr, &pData->offset, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkVertexInputAttributeDescription() {
    return "aa92c713cea863065cfae12ce0a4d7eedfe19fcb71c358a46581dae0";
}

foeResultSet binary_read_VkPipelineVertexInputStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineVertexInputStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineVertexInputStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    // uint32_t - vertexBindingDescriptionCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.vertexBindingDescriptionCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // uint32_t - vertexAttributeDescriptionCount
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.vertexAttributeDescriptionCount, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // VkPipelineVertexInputStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineVertexInputStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineVertexInputStateCreateFlags));
    readPtr += sizeof(VkPipelineVertexInputStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineVertexInputStateCreateFlags);

    // VkVertexInputBindingDescription* - pVertexBindingDescriptions[vertexBindingDescriptionCount]
    if (newData.vertexBindingDescriptionCount > 0) {
        newData.pVertexBindingDescriptions = (VkVertexInputBindingDescription *)calloc(
            newData.vertexBindingDescriptionCount, sizeof(VkVertexInputBindingDescription));
        if (newData.pVertexBindingDescriptions == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.vertexBindingDescriptionCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkVertexInputBindingDescription(
                readPtr, &subReadSize,
                (VkVertexInputBindingDescription *)newData.pVertexBindingDescriptions + i);
            if (result.value != FOE_SUCCESS)
                goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

    // VkVertexInputAttributeDescription* -
    // pVertexAttributeDescriptions[vertexAttributeDescriptionCount]
    if (newData.vertexAttributeDescriptionCount > 0) {
        newData.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription *)calloc(
            newData.vertexAttributeDescriptionCount, sizeof(VkVertexInputAttributeDescription));
        if (newData.pVertexAttributeDescriptions == NULL) {
            result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_OUT_OF_MEMORY);
            goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
        }

        for (size_t i = 0; i < newData.vertexAttributeDescriptionCount; ++i) {
            uint32_t subReadSize = bufferSizeLeft;
            result = binary_read_VkVertexInputAttributeDescription(
                readPtr, &subReadSize,
                (VkVertexInputAttributeDescription *)newData.pVertexAttributeDescriptions + i);
            if (result.value != FOE_SUCCESS)
                goto VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED;
            readPtr += subReadSize;
            bufferSizeLeft -= subReadSize;
        }
    }

VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineVertexInputStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineVertexInputStateCreateInfo(
    VkPipelineVertexInputStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // uint32_t - vertexBindingDescriptionCount
    writeSize += sizeof(uint32_t);

    // uint32_t - vertexAttributeDescriptionCount
    writeSize += sizeof(uint32_t);

    // VkPipelineVertexInputStateCreateFlags - flags
    writeSize += sizeof(VkPipelineVertexInputStateCreateFlags);

    // VkVertexInputBindingDescription* - pVertexBindingDescriptions[vertexBindingDescriptionCount]
    for (size_t i = 0; i < pData->vertexBindingDescriptionCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkVertexInputBindingDescription(pData->pVertexBindingDescriptions + i,
                                                     &subWriteSize, NULL);
        writeSize += subWriteSize;
    }

    // VkVertexInputAttributeDescription* -
    // pVertexAttributeDescriptions[vertexAttributeDescriptionCount]
    for (size_t i = 0; i < pData->vertexAttributeDescriptionCount; ++i) {
        uint32_t subWriteSize;
        binary_write_VkVertexInputAttributeDescription(pData->pVertexAttributeDescriptions + i,
                                                       &subWriteSize, NULL);
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

    // uint32_t - vertexBindingDescriptionCount
    memcpy(writePtr, &pData->vertexBindingDescriptionCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // uint32_t - vertexAttributeDescriptionCount
    memcpy(writePtr, &pData->vertexAttributeDescriptionCount, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // VkPipelineVertexInputStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineVertexInputStateCreateFlags));
    writePtr += sizeof(VkPipelineVertexInputStateCreateFlags);

    // VkVertexInputBindingDescription* - pVertexBindingDescriptions[vertexBindingDescriptionCount]
    for (size_t i = 0; i < pData->vertexBindingDescriptionCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkVertexInputBindingDescription(
            pData->pVertexBindingDescriptions + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // VkVertexInputAttributeDescription* -
    // pVertexAttributeDescriptions[vertexAttributeDescriptionCount]
    for (size_t i = 0; i < pData->vertexAttributeDescriptionCount; ++i) {
        uint32_t subWriteSize = *pWriteSize - (writePtr - (uint8_t *)pWriteBuffer);
        foeResultSet result = binary_write_VkVertexInputAttributeDescription(
            pData->pVertexAttributeDescriptions + i, &subWriteSize, writePtr);
        if (result.value != FOE_SUCCESS)
            return result;
        writePtr += subWriteSize;
    }

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineVertexInputStateCreateInfo() {
    return "c7f507efea057fd43967207f21b677135199d6baed986c368cddd4b8";
}

foeResultSet binary_read_VkPipelineInputAssemblyStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineInputAssemblyStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineInputAssemblyStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    // VkPipelineInputAssemblyStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineInputAssemblyStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineInputAssemblyStateCreateFlags));
    readPtr += sizeof(VkPipelineInputAssemblyStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineInputAssemblyStateCreateFlags);

    // VkPrimitiveTopology - topology
    if (bufferSizeLeft < sizeof(VkPrimitiveTopology)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.topology, readPtr, sizeof(VkPrimitiveTopology));
    readPtr += sizeof(VkPrimitiveTopology);
    bufferSizeLeft -= sizeof(VkPrimitiveTopology);

    // VkBool32 - primitiveRestartEnable
    if (bufferSizeLeft < sizeof(VkBool32)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.primitiveRestartEnable, readPtr, sizeof(VkBool32));
    readPtr += sizeof(VkBool32);
    bufferSizeLeft -= sizeof(VkBool32);

VK_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineInputAssemblyStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineInputAssemblyStateCreateInfo(
    VkPipelineInputAssemblyStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkPipelineInputAssemblyStateCreateFlags - flags
    writeSize += sizeof(VkPipelineInputAssemblyStateCreateFlags);

    // VkPrimitiveTopology - topology
    writeSize += sizeof(VkPrimitiveTopology);

    // VkBool32 - primitiveRestartEnable
    writeSize += sizeof(VkBool32);

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

    // VkPipelineInputAssemblyStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineInputAssemblyStateCreateFlags));
    writePtr += sizeof(VkPipelineInputAssemblyStateCreateFlags);

    // VkPrimitiveTopology - topology
    memcpy(writePtr, &pData->topology, sizeof(VkPrimitiveTopology));
    writePtr += sizeof(VkPrimitiveTopology);

    // VkBool32 - primitiveRestartEnable
    memcpy(writePtr, &pData->primitiveRestartEnable, sizeof(VkBool32));
    writePtr += sizeof(VkBool32);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineInputAssemblyStateCreateInfo() {
    return "9cec90fd8e87913e1fb172618bffcab3ddcb315abe591e273337d2df";
}

foeResultSet binary_read_VkPipelineTessellationStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineTessellationStateCreateInfo *pData) {
    foeResultSet result = foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
    uint8_t const *readPtr = (uint8_t const *)pReadBuffer;
    uint32_t bufferSizeLeft = *pReadSize;

    VkPipelineTessellationStateCreateInfo newData;
    memset(&newData, 0, sizeof(VkPipelineTessellationStateCreateInfo));

    // VkStructureType - sType
    newData.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    // VkPipelineTessellationStateCreateFlags - flags
    if (bufferSizeLeft < sizeof(VkPipelineTessellationStateCreateFlags)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_TESSELLATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.flags, readPtr, sizeof(VkPipelineTessellationStateCreateFlags));
    readPtr += sizeof(VkPipelineTessellationStateCreateFlags);
    bufferSizeLeft -= sizeof(VkPipelineTessellationStateCreateFlags);

    // uint32_t - patchControlPoints
    if (bufferSizeLeft < sizeof(uint32_t)) {
        result = foeBinaryResult_to_foeResultSet(FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        goto VK_PIPELINE_TESSELLATION_STATE_CREATE_INFO_READ_FAILED;
    }
    memcpy(&newData.patchControlPoints, readPtr, sizeof(uint32_t));
    readPtr += sizeof(uint32_t);
    bufferSizeLeft -= sizeof(uint32_t);

VK_PIPELINE_TESSELLATION_STATE_CREATE_INFO_READ_FAILED:
    if (result.value == FOE_SUCCESS) {
        // Copy content over and return total bytes read
        *pData = newData;
        *pReadSize = readPtr - (uint8_t const *)pReadBuffer;
    } else {
        cleanup_VkPipelineTessellationStateCreateInfo(&newData);
    }

    return result;
}

foeResultSet binary_write_VkPipelineTessellationStateCreateInfo(
    VkPipelineTessellationStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer) {
    // Calculate the required buffer size needed to write out the content before committing to do so
    uint32_t writeSize = 0;

    // VkPipelineTessellationStateCreateFlags - flags
    writeSize += sizeof(VkPipelineTessellationStateCreateFlags);

    // uint32_t - patchControlPoints
    writeSize += sizeof(uint32_t);

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

    // VkPipelineTessellationStateCreateFlags - flags
    memcpy(writePtr, &pData->flags, sizeof(VkPipelineTessellationStateCreateFlags));
    writePtr += sizeof(VkPipelineTessellationStateCreateFlags);

    // uint32_t - patchControlPoints
    memcpy(writePtr, &pData->patchControlPoints, sizeof(uint32_t));
    writePtr += sizeof(uint32_t);

    // Return bytes written and success result
    *pWriteSize = writePtr - (uint8_t *)pWriteBuffer;
    return foeBinaryResult_to_foeResultSet(FOE_BINARY_SUCCESS);
}

char const *binary_key_VkPipelineTessellationStateCreateInfo() {
    return "3860e9883304f62d551871b45626f7f16c028395c8db6f80958d158d";
}
