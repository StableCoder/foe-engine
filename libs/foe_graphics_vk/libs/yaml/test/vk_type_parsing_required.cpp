/*
    Copyright (C) 2020 George Cave.

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
#include <vulkan/vulkan.h>

TEST_CASE("Reading of Required Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    SECTION("Node exists and succeeds") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_NOTHROW(yaml_read_required<VkPrimitiveTopology>("topology", root, testVal));
        REQUIRE(testVal == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    }
    SECTION("Node does not exist and fails") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_THROWS_MATCHES(yaml_read_required<VkPrimitiveTopology>("topLOL", root, testVal),
                               foeYamlException,
                               Catch::Matchers::Contains(" - Required node not found"));
    }
    SECTION("Node exists with bad data fails") {
        YAML::Node root;
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: TOPLOL
)"));

        VkPrimitiveTopology testVal = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<VkPrimitiveTopology>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Contains(
                " - Could not parse node as 'VkPrimitiveTopology' with value of: "));
    }
}

TEST_CASE("Writing of Required Vulkan YAML Nodes", "[foe][yaml][vulkan]") {
    YAML::Node test;

    SECTION("Correct data succeeds") {
        yaml_write_required<VkPrimitiveTopology>("topology", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                                 test);

        YAML::Emitter emitter;
        emitter << test;

        REQUIRE(test["topology"]);
        REQUIRE(test["topology"].as<std::string>() == "TRIANGLE_LIST");

        REQUIRE(emitter.good());
        REQUIRE(std::string_view{emitter.c_str()} == "topology: TRIANGLE_LIST");
    }

    SECTION("Empty data fails if a node could not be generated") {
        REQUIRE_THROWS_MATCHES(
            yaml_write_required<VkBufferUsageFlagBits>("topology",
                                                       static_cast<VkBufferUsageFlagBits>(0), test),
            foeYamlException,
            Catch::Matchers::EndsWith(" - Failed to serialize node as 'VkBufferUsageFlagBits'"));
    }

    SECTION("Incorrect data fails") {
        REQUIRE_THROWS_MATCHES(
            yaml_write_required<VkQueueFlagBits>("topology", static_cast<VkQueueFlagBits>(3434),
                                                 test),
            foeYamlException,
            Catch::Matchers::EndsWith(" - Failed to serialize node as 'VkQueueFlagBits'"));
    }
}
