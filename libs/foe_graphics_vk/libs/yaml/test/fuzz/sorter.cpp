// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/yaml/exception.hpp>
#include <vulkan/vulkan.h>

#include <iostream>
#include <string_view>

typedef bool (*PFN_yamlRead)(std::string const &, YAML::Node const &, void *);

#define SORT_MACRO(X)                                                                              \
    if (std::string_view{argv[1]} == #X) {                                                         \
        sortType<X>(yamlData, testSet, (PFN_yamlRead)yaml_read_##X);                               \
    }

template <typename T>
void sortType(YAML::Node const &yamlData, std::string &testSet, PFN_yamlRead readFn) {
    T data;
    try {
        if (readFn("", yamlData, &data))
            testSet += '0';
        else
            testSet += '1';
    } catch (...) {
    }
    try {
        if (readFn("subNode", yamlData, &data))
            testSet += '2';
        else
            testSet += '3';
    } catch (...) {
    }
}

int main(int argc, char **argv) {
    if (argc < 3)
        return 1;

    YAML::Node yamlData;
    std::string testSet = "";

    try {
        yamlData = YAML::LoadFile(argv[2]);
    } catch (...) {
        return 1;
    }

    SORT_MACRO(VkPushConstantRange)
    SORT_MACRO(VkDescriptorSetLayoutBinding)
    SORT_MACRO(VkDescriptorSetLayoutCreateInfo)
    SORT_MACRO(VkStencilOpState)
    SORT_MACRO(VkPipelineRasterizationStateCreateInfo)
    SORT_MACRO(VkPipelineDepthStencilStateCreateInfo)
    SORT_MACRO(VkPipelineColorBlendAttachmentState)
    SORT_MACRO(VkPipelineColorBlendStateCreateInfo)
    SORT_MACRO(VkVertexInputBindingDescription)
    SORT_MACRO(VkVertexInputAttributeDescription)
    SORT_MACRO(VkPipelineVertexInputStateCreateInfo)
    SORT_MACRO(VkPipelineInputAssemblyStateCreateInfo)
    SORT_MACRO(VkPipelineTessellationStateCreateInfo)

    if (testSet.empty())
        testSet = 'n';

    std::cout << testSet << std::endl;

    return 0;
}