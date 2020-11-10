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
#include <foe/yaml/pod_parsing.hpp>
#include <vk_equality_checks.hpp>
#include <vulkan/vulkan.h>
#include <yaml-cpp/emitter.h>

#include <string>

TEST_CASE("yaml_read_required - VkStencilOpState", "[foe][yaml][vulkan]") {
    SECTION("Exists as empty node and is successful") {
        std::string testStr = R"(test_stencil_op_state: ~)";
        YAML::Node testNode;
        VkStencilOpState data{};

        REQUIRE_NOTHROW(testNode = YAML::Load(testStr));

        REQUIRE_NOTHROW(yaml_read_required("test_stencil_op_state", testNode, data));
        REQUIRE(data == VkStencilOpState{});
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

        REQUIRE(data == VkStencilOpState{
                            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
                            .depthFailOp = VK_STENCIL_OP_REPLACE,
                            .compareMask = 152,
                            .reference = 12,
                        });
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
        REQUIRE(data == VkStencilOpState{});
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

        REQUIRE(data == VkStencilOpState{
                            .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
                            .depthFailOp = VK_STENCIL_OP_REPLACE,
                            .compareMask = 152,
                            .reference = 12,
                        });
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