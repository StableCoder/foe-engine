// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP
#define FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP

#include <foe/graphics/vk/yaml/export.h>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

#include <string>

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPushConstantRange(std::string const &nodeName,
                                   YAML::Node const &node,
                                   VkPushConstantRange &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPushConstantRange(std::string const &nodeName,
                                    VkPushConstantRange const &data,
                                    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkDescriptorSetLayoutBinding(std::string const &nodeName,
                                            YAML::Node const &node,
                                            VkDescriptorSetLayoutBinding &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkDescriptorSetLayoutBinding(std::string const &nodeName,
                                             VkDescriptorSetLayoutBinding const &data,
                                             YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkDescriptorSetLayoutCreateInfo(std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkDescriptorSetLayoutCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkDescriptorSetLayoutCreateInfo(std::string const &nodeName,
                                                VkDescriptorSetLayoutCreateInfo const &data,
                                                YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkStencilOpState(std::string const &nodeName,
                                YAML::Node const &node,
                                VkStencilOpState &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkStencilOpState(std::string const &nodeName,
                                 VkStencilOpState const &data,
                                 YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineRasterizationStateCreateInfo(std::string const &nodeName,
                                                      YAML::Node const &node,
                                                      VkPipelineRasterizationStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineRasterizationStateCreateInfo(
    std::string const &nodeName,
    VkPipelineRasterizationStateCreateInfo const &data,
    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineDepthStencilStateCreateInfo(std::string const &nodeName,
                                                     YAML::Node const &node,
                                                     VkPipelineDepthStencilStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineDepthStencilStateCreateInfo(
    std::string const &nodeName,
    VkPipelineDepthStencilStateCreateInfo const &data,
    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineColorBlendAttachmentState(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   VkPipelineColorBlendAttachmentState &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineColorBlendAttachmentState(std::string const &nodeName,
                                                    VkPipelineColorBlendAttachmentState const &data,
                                                    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineColorBlendStateCreateInfo(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   VkPipelineColorBlendStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineColorBlendStateCreateInfo(std::string const &nodeName,
                                                    VkPipelineColorBlendStateCreateInfo const &data,
                                                    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkVertexInputBindingDescription(std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkVertexInputBindingDescription &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkVertexInputBindingDescription(std::string const &nodeName,
                                                VkVertexInputBindingDescription const &data,
                                                YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkVertexInputAttributeDescription(std::string const &nodeName,
                                                 YAML::Node const &node,
                                                 VkVertexInputAttributeDescription &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkVertexInputAttributeDescription(std::string const &nodeName,
                                                  VkVertexInputAttributeDescription const &data,
                                                  YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineVertexInputStateCreateInfo(std::string const &nodeName,
                                                    YAML::Node const &node,
                                                    VkPipelineVertexInputStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineVertexInputStateCreateInfo(
    std::string const &nodeName,
    VkPipelineVertexInputStateCreateInfo const &data,
    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineInputAssemblyStateCreateInfo(std::string const &nodeName,
                                                      YAML::Node const &node,
                                                      VkPipelineInputAssemblyStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineInputAssemblyStateCreateInfo(
    std::string const &nodeName,
    VkPipelineInputAssemblyStateCreateInfo const &data,
    YAML::Node &node);

FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkPipelineTessellationStateCreateInfo(std::string const &nodeName,
                                                     YAML::Node const &node,
                                                     VkPipelineTessellationStateCreateInfo &data);

FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkPipelineTessellationStateCreateInfo(
    std::string const &nodeName,
    VkPipelineTessellationStateCreateInfo const &data,
    YAML::Node &node);

#endif // FOE_GRAPHICS_VK_YAML_VK_STRUCTS_HPP
