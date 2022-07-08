// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/yaml/vk_enum.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vulkan/vulkan.h>

TEST_CASE("Reading of Required Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    SECTION("Node exists and succeeds") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_NOTHROW(
            yaml_read_required_VkEnum("VkPrimitiveTopology", "topology", root, testVal));
        REQUIRE(testVal == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    }
    SECTION("Node does not exist and fails") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required_VkEnum("VkPrimitiveTopology", "topLOL", root, testVal),
            foeYamlException, Catch::Matchers::Contains(" - Required node not found"));
    }
    SECTION("Node exists with bad data fails") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: TOPLOL
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required_VkEnum("VkPrimitiveTopology", "topology", root, testVal),
            foeYamlException,
            Catch::Matchers::Contains(
                " - Could not parse node as 'VkPrimitiveTopology' with value of: "));
    }
}

TEST_CASE("Writing of Required Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    YAML::Node test;

    SECTION("Correct data succeeds") {
        yaml_write_required_VkEnum("VkPrimitiveTopology", "topology",
                                   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, test);

        YAML::Emitter emitter;
        emitter << test;

        REQUIRE(test["topology"]);
        REQUIRE(test["topology"].as<std::string>() == "TRIANGLE_LIST");

        REQUIRE(emitter.good());
        REQUIRE(std::string_view{emitter.c_str()} == "topology: TRIANGLE_LIST");
    }

    SECTION("Incorrect data fails") {
        REQUIRE_THROWS_MATCHES(
            yaml_write_required_VkEnum("VkQueueFlagBits", "topology",
                                       static_cast<VkQueueFlagBits>(3434), test),
            foeYamlException,
            Catch::Matchers::EndsWith(" - Failed to serialize node as 'VkQueueFlagBits'"));
    }
}
