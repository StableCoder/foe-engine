#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vulkan/vulkan.h>

#include <string_view>

#define FUZZ_MACRO(X)                                                                              \
    X data;                                                                                        \
    try {                                                                                          \
        yamlData = YAML::LoadFile(argv[2]);                                                        \
    } catch (...) {                                                                                \
        continue;                                                                                  \
    }                                                                                              \
    try {                                                                                          \
        yaml_read_optional("", yamlData, data);                                                    \
    } catch (...) {                                                                                \
    }                                                                                              \
    try {                                                                                          \
        yaml_read_optional("subNode", yamlData, data);                                             \
    } catch (...) {                                                                                \
    }                                                                                              \
    try {                                                                                          \
        yaml_read_required("", yamlData, data);                                                    \
    } catch (...) {                                                                                \
    }                                                                                              \
    try {                                                                                          \
        yaml_read_required("subNode", yamlData, data);                                             \
    } catch (...) {                                                                                \
    }

int main(int argc, char **argv) {
    if (argc < 3)
        return 1;

    YAML::Node yamlData;

    if (std::string_view{argv[1]} == "VkPushConstantRange") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPushConstantRange)
        }
    }
    if (std::string_view{argv[1]} == "VkDescriptorSetLayoutBinding") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkDescriptorSetLayoutBinding)
        }
    }
    if (std::string_view{argv[1]} == "VkDescriptorSetLayoutCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkDescriptorSetLayoutCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkStencilOpState") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkStencilOpState)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineRasterizationStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineRasterizationStateCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineDepthStencilStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineDepthStencilStateCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineColorBlendAttachmentState") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineColorBlendAttachmentState)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineColorBlendStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineColorBlendStateCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkVertexInputBindingDescription") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkVertexInputBindingDescription)
        }
    }
    if (std::string_view{argv[1]} == "VkVertexInputAttributeDescription") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkVertexInputAttributeDescription)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineVertexInputStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineVertexInputStateCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineInputAssemblyStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineInputAssemblyStateCreateInfo)
        }
    }
    if (std::string_view{argv[1]} == "VkPipelineTessellationStateCreateInfo") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO(VkPipelineTessellationStateCreateInfo)
        }
    }

    return 0;
}