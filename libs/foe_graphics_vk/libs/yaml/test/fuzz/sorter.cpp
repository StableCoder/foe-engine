#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vulkan/vulkan.h>

#include <iostream>
#include <string_view>

#define SORT_MACRO(X)                                                                              \
    if (std::string_view{argv[1]} == #X) {                                                         \
        sortType<X>(yamlData, testSet);                                                            \
    }

template <typename T>
void sortType(YAML::Node const &yamlData, std::string &testSet) {
    T data;
    try {
        if (yaml_read_optional("", yamlData, data))
            testSet += '0';
        else
            testSet += '1';
    } catch (...) {
    }
    try {
        if (yaml_read_optional("subNode", yamlData, data))
            testSet += '2';
        else
            testSet += '3';
    } catch (...) {
    }
    try {
        yaml_read_required("", yamlData, data);
        testSet += '4';
    } catch (...) {
    }
    try {
        yaml_read_required("subNode", yamlData, data);
        testSet += '5';
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