// Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/yaml/exception.hpp>
#include <vk_struct_compare.h>
#include <yaml-cpp/emitter.h>

#include <string>

TEST_CASE("yaml_read - VkStencilOpState", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_stencil_op_state: ~)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_VkStencilOpState("test_stencil_op_state", testNode, data));
        VkStencilOpState cmpData{};
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_stencil_op_state:
  fail_op: DECREMENT_AND_CLAMP
  depth_fail_op: REPLACE
  compare_mask: 152
  reference: 12)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_VkStencilOpState("test_stencil_op_state", testNode, data));

        VkStencilOpState cmpData{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };
        REQUIRE(compare_VkStencilOpState(&data, &cmpData));
    }
}

TEST_CASE("yaml_write - VkStencilOpState", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkStencilOpState data{};

        REQUIRE_NOTHROW(yaml_write_VkStencilOpState("test_stencil_op_state", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_stencil_op_state:
  fail_op: KEEP
  pass_op: KEEP
  depth_fail_op: KEEP
  compare_op: NEVER)");
    }

    SECTION("Custom-filled data") {
        VkStencilOpState data{
            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            .depthFailOp = VK_STENCIL_OP_REPLACE,
            .compareMask = 152,
            .reference = 12,
        };

        REQUIRE_NOTHROW(yaml_write_VkStencilOpState("test_stencil_op_state", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_stencil_op_state:
  fail_op: DECREMENT_AND_CLAMP
  pass_op: KEEP
  depth_fail_op: REPLACE
  compare_op: NEVER
  compare_mask: 152
  reference: 12)");
    }
}