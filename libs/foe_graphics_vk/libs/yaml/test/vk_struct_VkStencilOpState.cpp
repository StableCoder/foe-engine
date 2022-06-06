// Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_struct_compare.h>
#include <yaml-cpp/emitter.h>

#include <string>

TEST_CASE("yaml_read_required - VkStencilOpState", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_stencil_op_state: ~)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_required("test_stencil_op_state", testNode, data));
        VkStencilOpState cmpData{};
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_stencil_op_state:
  failOp: DECREMENT_AND_CLAMP
  depthFailOp: REPLACE
  compareMask: 152
  reference: 12)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_required("test_stencil_op_state", testNode, data));

        VkStencilOpState cmpData{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }

    SECTION("Node doesn't exist and it throws an appropriate exception") {
        YAML::Node testNode;
        VkStencilOpState data;

        REQUIRE_NOTHROW(testNode = YAML::Load(R"(test_stencil_op_stateasdasdasdasd: ~)"));

        REQUIRE_THROWS_MATCHES(yaml_read_required("test_stencil_op_state", testNode, data),
                               foeYamlException,
                               Catch::Matchers::Contains(" - Required node not found"));
    }
}

TEST_CASE("yaml_read_optional - VkStencilOpState", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_stencil_op_state: ~)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_optional("test_stencil_op_state", testNode, data));
        VkStencilOpState cmpData{};
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_stencil_op_state:
  failOp: DECREMENT_AND_CLAMP
  depthFailOp: REPLACE
  compareMask: 152
  reference: 12)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_optional("test_stencil_op_state", testNode, data));

        VkStencilOpState cmpData{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }

    SECTION("Node doesn't exist and returns without exception") {
        YAML::Node testNode;
        VkStencilOpState data;

        REQUIRE_NOTHROW(testNode = YAML::Load(R"(test_stencil_op_stateasdasdasdasd: ~)"));

        REQUIRE_NOTHROW(yaml_read_optional("test_stencil_op_state", testNode, data));
    }
}

TEST_CASE("yaml_write_required - VkStencilOpState", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkStencilOpState data{};

        REQUIRE_NOTHROW(yaml_write_required("test_stencil_op_state", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_stencil_op_state:
  failOp: KEEP
  passOp: KEEP
  depthFailOp: KEEP
  compareOp: NEVER
  compareMask: 0
  writeMask: 0
  reference: 0)");
    }

    SECTION("Custom-filled data") {
        VkStencilOpState data{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };

        REQUIRE_NOTHROW(yaml_write_required("test_stencil_op_state", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_stencil_op_state:
  failOp: DECREMENT_AND_CLAMP
  passOp: KEEP
  depthFailOp: REPLACE
  compareOp: NEVER
  compareMask: 152
  writeMask: 0
  reference: 12)");
    }
}

TEST_CASE("yaml_write_optional - VkStencilOpState", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkStencilOpState data{};

        REQUIRE_NOTHROW(
            yaml_write_optional("test_stencil_op_state", VkStencilOpState{}, data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"()");
    }

    SECTION("Custom-filled data") {
        VkStencilOpState data{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };

        REQUIRE_NOTHROW(
            yaml_write_optional("test_stencil_op_state", VkStencilOpState{}, data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_stencil_op_state:
  failOp: DECREMENT_AND_CLAMP
  depthFailOp: REPLACE
  compareMask: 152
  reference: 12)");
    }
}