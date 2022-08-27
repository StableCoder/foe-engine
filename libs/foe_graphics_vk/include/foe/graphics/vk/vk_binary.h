// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP
#define FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP

#include <foe/graphics/export.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_EXPORT foeResultSet binary_read_VkPushConstantRange(void const *pReadBuffer,
                                                            uint32_t *pReadSize,
                                                            VkPushConstantRange *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPushConstantRange(VkPushConstantRange const *pData,
                                                             uint32_t *pWriteSize,
                                                             void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPushConstantRange();

FOE_GFX_EXPORT foeResultSet binary_read_VkDescriptorSetLayoutBinding(
    void const *pReadBuffer, uint32_t *pReadSize, VkDescriptorSetLayoutBinding *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkDescriptorSetLayoutBinding(
    VkDescriptorSetLayoutBinding const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkDescriptorSetLayoutBinding();

FOE_GFX_EXPORT foeResultSet binary_read_VkDescriptorSetLayoutCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkDescriptorSetLayoutCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkDescriptorSetLayoutCreateInfo(
    VkDescriptorSetLayoutCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkDescriptorSetLayoutCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkStencilOpState(void const *pReadBuffer,
                                                         uint32_t *pReadSize,
                                                         VkStencilOpState *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkStencilOpState(VkStencilOpState const *pData,
                                                          uint32_t *pWriteSize,
                                                          void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkStencilOpState();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineRasterizationStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineRasterizationStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineRasterizationStateCreateInfo(
    VkPipelineRasterizationStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineRasterizationStateCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineDepthStencilStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineDepthStencilStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineDepthStencilStateCreateInfo(
    VkPipelineDepthStencilStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineDepthStencilStateCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineColorBlendAttachmentState(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineColorBlendAttachmentState *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineColorBlendAttachmentState(
    VkPipelineColorBlendAttachmentState const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineColorBlendAttachmentState();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineColorBlendStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineColorBlendStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineColorBlendStateCreateInfo(
    VkPipelineColorBlendStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineColorBlendStateCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkVertexInputBindingDescription(
    void const *pReadBuffer, uint32_t *pReadSize, VkVertexInputBindingDescription *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkVertexInputBindingDescription(
    VkVertexInputBindingDescription const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkVertexInputBindingDescription();

FOE_GFX_EXPORT foeResultSet binary_read_VkVertexInputAttributeDescription(
    void const *pReadBuffer, uint32_t *pReadSize, VkVertexInputAttributeDescription *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkVertexInputAttributeDescription(
    VkVertexInputAttributeDescription const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkVertexInputAttributeDescription();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineVertexInputStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineVertexInputStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineVertexInputStateCreateInfo(
    VkPipelineVertexInputStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineVertexInputStateCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineInputAssemblyStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineInputAssemblyStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineInputAssemblyStateCreateInfo(
    VkPipelineInputAssemblyStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineInputAssemblyStateCreateInfo();

FOE_GFX_EXPORT foeResultSet binary_read_VkPipelineTessellationStateCreateInfo(
    void const *pReadBuffer, uint32_t *pReadSize, VkPipelineTessellationStateCreateInfo *pData);

FOE_GFX_EXPORT foeResultSet binary_write_VkPipelineTessellationStateCreateInfo(
    VkPipelineTessellationStateCreateInfo const *pData, uint32_t *pWriteSize, void *pWriteBuffer);

FOE_GFX_EXPORT char const *binary_key_VkPipelineTessellationStateCreateInfo();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP
