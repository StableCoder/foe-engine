/*
    Copyright (C) 2022 George Cave - gcave@stablecoder.ca

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_struct_compare.h>
#include <vulkan/vulkan.h>

#include <yaml-cpp/emitter.h>

#include <filesystem>
#include <string>

namespace {

template <typename T>
void runTest(std::filesystem::path path, bool (*pCompareFn)(T const *, T const *)) {
    T data;

    if (!std::filesystem::exists(path))
        return;

    for (auto const &it : std::filesystem::directory_iterator{path}) {
        YAML::Node yamlData = YAML::LoadFile(it.path().string());

        // Determine the test set
        std::string setStr = it.path().string();
        setStr = setStr.substr(setStr.find_last_of('-'));
        setStr = setStr.substr(1, setStr.find_last_of('.') - 1);

        // Optional Case
        if (setStr.find('0') != std::string::npos)
            CHECK(yaml_read_optional("", yamlData, data));
        else if (setStr.find('1') != std::string::npos)
            CHECK_FALSE(yaml_read_optional("", yamlData, data));
        else
            CHECK_THROWS(yaml_read_optional("", yamlData, data));

        // Optional Sub Case
        if (setStr.find('2') != std::string::npos)
            CHECK(yaml_read_optional("subNode", yamlData, data));
        else if (setStr.find('3') != std::string::npos)
            CHECK_FALSE(yaml_read_optional("subNode", yamlData, data));
        else
            CHECK_THROWS(yaml_read_optional("subNode", yamlData, data));

        // Required Case
        if (setStr.find('4') != std::string::npos)
            CHECK_NOTHROW(yaml_read_required("", yamlData, data));
        else
            CHECK_THROWS(yaml_read_required("", yamlData, data));

        // Required Sub Case
        if (setStr.find('5') != std::string::npos)
            CHECK_NOTHROW(yaml_read_required("subNode", yamlData, data));
        else
            CHECK_THROWS(yaml_read_required("subNode", yamlData, data));
    }
}

} // namespace

#define FUZZ_TEST_MACRO(X, Y)                                                                      \
    std::filesystem::path inputDir{X};                                                             \
    inputDir /= #Y;                                                                                \
    runTest<Y>(inputDir, compare_##Y);

TEST_CASE("VkPushConstantRange fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPushConstantRange);
}

TEST_CASE("VkDescriptorSetLayoutBinding fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkDescriptorSetLayoutBinding);
}

TEST_CASE("VkDescriptorSetLayoutCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkDescriptorSetLayoutCreateInfo);
}

TEST_CASE("VkStencilOpState fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkStencilOpState);
}

TEST_CASE("VkPipelineRasterizationStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineRasterizationStateCreateInfo);
}

TEST_CASE("VkPipelineDepthStencilStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineDepthStencilStateCreateInfo);
}

TEST_CASE("VkPipelineColorBlendAttachmentState fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineColorBlendAttachmentState);
}

TEST_CASE("VkPipelineColorBlendStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineColorBlendStateCreateInfo);
}

TEST_CASE("VkVertexInputBindingDescription fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkVertexInputBindingDescription);
}

TEST_CASE("VkVertexInputAttributeDescription fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkVertexInputAttributeDescription);
}

TEST_CASE("VkPipelineVertexInputStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineVertexInputStateCreateInfo);
}

TEST_CASE("VkPipelineInputAssemblyStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineInputAssemblyStateCreateInfo);
}

TEST_CASE("VkPipelineTessellationStateCreateInfo fuzzed input") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, VkPipelineTessellationStateCreateInfo);
}