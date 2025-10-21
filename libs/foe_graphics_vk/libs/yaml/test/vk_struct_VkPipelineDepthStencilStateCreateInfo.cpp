// Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/external/vk_struct_compare.h>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/yaml/exception.hpp>
#include <yaml-cpp/emitter.h>

#include <iostream>
#include <string>

namespace {

constexpr VkPipelineDepthStencilStateCreateInfo cFilledData{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthCompareOp = VK_COMPARE_OP_GREATER,
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
    .maxDepthBounds = 1024.5f,
};

} // namespace

TEST_CASE("yaml_read - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_struct: ~)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE(yaml_read_VkPipelineDepthStencilStateCreateInfo("test_struct", testNode, data));

        VkPipelineDepthStencilStateCreateInfo cmpData{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };
        REQUIRE(compare_VkPipelineDepthStencilStateCreateInfo(&data, &cmpData));
    }

    SECTION("Exists as node with data and is successful") {
        std::string testStr = R"(test_struct:
  depth_compare_op: GREATER
  front:
    fail_op: INVERT
    write_mask: 12
  back:
    fail_op: REPLACE
    write_mask: 10
  max_depth_bounds: 1024.5)";
        YAML::Node testNode;
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(
            yaml_read_VkPipelineDepthStencilStateCreateInfo("test_struct", testNode, data));

        REQUIRE(compare_VkPipelineDepthStencilStateCreateInfo(&data, &cFilledData));
    }
}

TEST_CASE("yaml_write - VkPipelineDepthStencilStateCreateInfo", "[foe][yaml][vulkan]") {
    YAML::Node dataNode;

    SECTION("Zero-filled struct") {
        VkPipelineDepthStencilStateCreateInfo data{};

        REQUIRE_NOTHROW(
            yaml_write_VkPipelineDepthStencilStateCreateInfo("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  depth_compare_op: NEVER)");
    }

    SECTION("Filled-in sType and pNext isn't written out") {
        VkPipelineDepthStencilStateCreateInfo data{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = &data,
        };

        REQUIRE_NOTHROW(
            yaml_write_VkPipelineDepthStencilStateCreateInfo("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  depth_compare_op: NEVER)");
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

        REQUIRE_NOTHROW(
            yaml_write_VkPipelineDepthStencilStateCreateInfo("test_struct", data, dataNode));

        YAML::Emitter emitter;
        emitter << dataNode;

        REQUIRE(std::string{emitter.c_str()} == R"(test_struct:
  depth_compare_op: GREATER
  front:
    fail_op: INVERT
    pass_op: KEEP
    depth_fail_op: KEEP
    compare_op: NEVER
    write_mask: 12
  back:
    fail_op: REPLACE
    pass_op: KEEP
    depth_fail_op: KEEP
    compare_op: NEVER
    write_mask: 10
  max_depth_bounds: 1024.5)");
    }
}
