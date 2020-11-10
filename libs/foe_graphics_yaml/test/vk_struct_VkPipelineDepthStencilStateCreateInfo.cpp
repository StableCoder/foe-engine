/*
    Copyright (C) 2020 George Cave - gcave@stablecoder.ca

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
#include <vk_equality_checks.hpp>
#include <vulkan/vulkan.h>
#include <yaml-cpp/emitter.h>

#include <iostream>
#include <string>

constexpr VkPipelineDepthStencilStateCreateInfo cFilledData{.depthCompareOp = VK_COMPARE_OP_GREATER,
                                                            .front =
                                                                VkStencilOpState{
                                                                    .failOp = VK_STENCIL_OP_INVERT,
                                                                    .writeMask = 12,
                                                                },
                                                            .back =
                                                                VkStencilOpState{
                                                                    .failOp = VK_STENCIL_OP_REPLACE,
                                                                    .writeMask = 10,
                                                                },
                                                            .maxDepthBounds = 1024.5f};

TEST_CASE("yaml_read_required - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_struct: ~)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_required("test_struct", testNode, data));
        REQUIRE(data == VkPipelineDepthStencilStateCreateInfo{});
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_struct:
  depthCompareOp: GREATER
  front:
    failOp: INVERT
    writeMask: 12
  back:
    failOp: REPLACE
    writeMask: 10
  maxDepthBounds: 1024.5)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_required("test_struct", testNode, data));

        REQUIRE(data == cFilledData);
    }

    SECTION("Node doesn't exist and it throws an appropriate exception") {
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data;

        REQUIRE_NOTHROW(testNode = YAML::Load(R"(test_struct_adsdads: ~)"));

        REQUIRE_THROWS_MATCHES(yaml_read_required("test_struct", testNode, data), foeYamlException,
                               Catch::Matchers::Contains(" - Required node not found"));
    }
}

TEST_CASE("yaml_read_optional - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_struct: ~)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_optional("test_struct", testNode, data));

        REQUIRE(data == VkPipelineDepthStencilStateCreateInfo{});
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_struct:
  depthCompareOp: GREATER
  front:
    failOp: INVERT
    writeMask: 12
  back:
    failOp: REPLACE
    writeMask: 10
  maxDepthBounds: 1024.5)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_optional("test_struct", testNode, data));

        REQUIRE(data == cFilledData);
    }

    SECTION("Node doesn't exist and it doesn't throw an exception") {
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{cFilledData};

        REQUIRE_NOTHROW(testNode = YAML::Load(R"(test_struct_adsdads: ~)"));

        REQUIRE_NOTHROW(yaml_read_optional("test_struct", testNode, data));

        REQUIRE(data == cFilledData);
    }
}

TEST_CASE("yaml_write_required - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(yaml_write_required("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  flags: ""
  depthTestEnable: 0
  depthWriteEnable: 0
  depthCompareOp: NEVER
  depthBoundsTestEnable: 0
  stencilTestEnable: 0
  front:
    failOp: KEEP
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 0
    reference: 0
  back:
    failOp: KEEP
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 0
    reference: 0
  minDepthBounds: 0
  maxDepthBounds: 0)");
    }

    SECTION("Filled-in sType and pNext isn't written out") {
        VkPipelineDepthStencilStateCreateInfo data{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = &data,
        };

        REQUIRE_NOTHROW(yaml_write_required("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  flags: ""
  depthTestEnable: 0
  depthWriteEnable: 0
  depthCompareOp: NEVER
  depthBoundsTestEnable: 0
  stencilTestEnable: 0
  front:
    failOp: KEEP
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 0
    reference: 0
  back:
    failOp: KEEP
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 0
    reference: 0
  minDepthBounds: 0
  maxDepthBounds: 0)");
    }

    SECTION("Custom data written out") {
        VkPipelineDepthStencilStateCreateInfo data{.depthCompareOp = VK_COMPARE_OP_GREATER,
                                                   .front =
                                                       VkStencilOpState{
                                                           .failOp = VK_STENCIL_OP_INVERT,
                                                           .writeMask = 12,
                                                       },
                                                   .back =
                                                       VkStencilOpState{
                                                           .failOp = VK_STENCIL_OP_REPLACE,
                                                           .writeMask = 10,
                                                       },
                                                   .maxDepthBounds = 1024.5f};

        REQUIRE_NOTHROW(yaml_write_required("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  flags: ""
  depthTestEnable: 0
  depthWriteEnable: 0
  depthCompareOp: GREATER
  depthBoundsTestEnable: 0
  stencilTestEnable: 0
  front:
    failOp: INVERT
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 12
    reference: 0
  back:
    failOp: REPLACE
    passOp: KEEP
    depthFailOp: KEEP
    compareOp: NEVER
    compareMask: 0
    writeMask: 10
    reference: 0
  minDepthBounds: 0
  maxDepthBounds: 1024.5)");
    }
}

TEST_CASE("yaml_write_optional - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_FALSE(yaml_write_optional("test_struct", VkPipelineDepthStencilStateCreateInfo{},
                                          data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"()");
    }

    SECTION("Filled-in sType and pNext isn't written out") {
        VkPipelineDepthStencilStateCreateInfo data{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = &data,
        };

        REQUIRE_FALSE(yaml_write_optional("test_struct", VkPipelineDepthStencilStateCreateInfo{},
                                          data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"()");
    }

    SECTION("Custom data written out") {
        VkPipelineDepthStencilStateCreateInfo data{cFilledData};

        REQUIRE(yaml_write_optional("test_struct", VkPipelineDepthStencilStateCreateInfo{}, data,
                                    dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  depthCompareOp: GREATER
  front:
    failOp: INVERT
    writeMask: 12
  back:
    failOp: REPLACE
    writeMask: 10
  maxDepthBounds: 1024.5)");
    }
}