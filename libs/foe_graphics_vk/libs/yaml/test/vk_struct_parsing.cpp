// Copyright (C) 2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/yaml/exception.hpp>
#include <vk_struct_compare.h>
#include <yaml-cpp/yaml.h>

#include <yaml-cpp/emitter.h>

#include <filesystem>
#include <string>

namespace {

typedef bool (*PFN_yamlRead)(std::string const &, YAML::Node const &, void *);

template <typename T>
void runTest(std::filesystem::path path,
             bool (*pCompareFn)(T const *, T const *),
             PFN_yamlRead pReadFn) {
    T data;

    if (!std::filesystem::exists(path))
        return;

    for (auto const &it : std::filesystem::directory_iterator{path}) {
        YAML::Node yamlData = YAML::LoadFile(it.path().string());

        // Determine the test set
        std::string setStr = it.path().string();
        setStr = setStr.substr(setStr.find_last_of('-'));
        setStr = setStr.substr(1, setStr.find_last_of('.') - 1);

        // Same-Node Case
        if (setStr.find('0') != std::string::npos)
            CHECK(pReadFn("", yamlData, &data));
        else if (setStr.find('1') != std::string::npos)
            CHECK_FALSE(pReadFn("", yamlData, &data));
        else
            CHECK_THROWS(pReadFn("", yamlData, &data));

        // Sub-Node Case
        if (setStr.find('2') != std::string::npos)
            CHECK(pReadFn("subNode", yamlData, &data));
        else if (setStr.find('3') != std::string::npos)
            CHECK_FALSE(pReadFn("subNode", yamlData, &data));
        else
            CHECK_THROWS(pReadFn("subNode", yamlData, &data));
    }
}

} // namespace

#define FUZZ_TEST_MACRO(X, Y)                                                                      \
    std::filesystem::path inputDir{X};                                                             \
    inputDir /= #Y;                                                                                \
    runTest<Y>(inputDir, compare_##Y, (PFN_yamlRead)yaml_read_##Y);

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
