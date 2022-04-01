/*
    Copyright (C) 2020-2022 George Cave.

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
#include <foe/graphics/vk/yaml/vk_enum.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vulkan/vulkan.h>

TEST_CASE("Reading of Optional Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    SECTION("Node exists and succeeds") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        REQUIRE_NOTHROW(
            yaml_read_optional_VkEnum("VkPrimitiveTopology", "topology", root, testVal));
        REQUIRE(testVal == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    }
    SECTION("Node does not exist and doesn't update the value") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(toplolol: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        REQUIRE_NOTHROW(yaml_read_optional_VkEnum("VkPrimitiveTopology", "topLOL", root, testVal));
        REQUIRE(testVal == VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    }
    SECTION("Node exists with bad data fails") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: TOPLOL
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        REQUIRE_THROWS_MATCHES(
            yaml_read_optional_VkEnum("VkPrimitiveTopology", "topology", root, testVal),
            foeYamlException,
            Catch::Matchers::Contains(
                " - Could not parse node as 'VkPrimitiveTopology' with value of: "));
    }
}

TEST_CASE("Writing of Optional Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    YAML::Node test;

    SECTION("Correct data succeeds") {
        yaml_write_optional_VkEnum("VkPrimitiveTopology", "topology",
                                   VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, test);

        YAML::Emitter emitter;
        emitter << test;

        REQUIRE(test["topology"]);
        REQUIRE(test["topology"].as<std::string>() == "TRIANGLE_LIST");

        REQUIRE(emitter.good());
        REQUIRE(std::string_view{emitter.c_str()} == "topology: TRIANGLE_LIST");
    }

    SECTION("When a value is the same as the 'default', it doesn't write the node") {
        yaml_write_optional_VkEnum("VkPrimitiveTopology", "topology",
                                   VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                   test);

        YAML::Emitter emitter;
        emitter << test;

        REQUIRE_FALSE(test["topology"]);
    }

    SECTION("Incorrect data fails") {
        REQUIRE_THROWS_MATCHES(
            yaml_write_optional_VkEnum("VkPrimitiveTopology", "topology",
                                       VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                       static_cast<VkPrimitiveTopology>(3434), test),
            foeYamlException,
            Catch::Matchers::EndsWith(" - Failed to serialize node as 'VkPrimitiveTopology'"));
    }
}