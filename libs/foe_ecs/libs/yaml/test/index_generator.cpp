/*
    Copyright (C) 2020-2021 George Cave - gcave@stablecoder.ca

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
#include <foe/ecs/index_generator.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/yaml/exception.hpp>

TEST_CASE("yaml_read_index_generator - Success Case", "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [8, 4, 10])"));

    foeIdIndexGenerator testGenerator{"", 0};
    REQUIRE_NOTHROW(yaml_read_index_generator("", root, testGenerator));

    CHECK(testGenerator.generate() == 8);
    CHECK(testGenerator.generate() == 4);
    CHECK(testGenerator.generate() == 10);
    CHECK(testGenerator.generate() == 15);
    CHECK(testGenerator.generate() == 16);
}

TEST_CASE("yaml_write_index_generator", "[foe][ecs][yaml][IndexGenerator]") {
    foeIdIndexGenerator indexGen{"", 0};

    for (int i = 0; i < 15; ++i) {
        indexGen.generate();
    }
    indexGen.free(8);
    indexGen.free(4);
    indexGen.free(10);

    YAML::Node testNode;
    REQUIRE_NOTHROW(yaml_write_index_generator("", indexGen, testNode));

    foeIdIndexGenerator testGenerator{"", 0};
    REQUIRE_NOTHROW(yaml_read_index_generator("", testNode, testGenerator));

    CHECK(testGenerator.generate() == 8);
    CHECK(testGenerator.generate() == 4);
    CHECK(testGenerator.generate() == 10);
    CHECK(testGenerator.generate() == 16);
    CHECK(testGenerator.generate() == 17);
}

TEST_CASE("yaml_read_index_generator - Missing 'next_free_index' node",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(recycled_indices: [8, 4, 10])"));

    foeIdIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(yaml_read_index_generator("", root, temp), foeYamlException,
                           Catch::Matchers::Equals(
                               "next_free_index - Required node not found to parse as 'uint32_t'"));
}

TEST_CASE("yaml_read_index_generator - 'next_free_index' node not integer",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: fifteen
recycled_indices: [8, 4, 10])"));

    foeIdIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator("", root, temp), foeYamlException,
        Catch::Matchers::Equals("next_free_index - Could not parse Map-type node as 'uint32_t'"));
}

TEST_CASE("yaml_read_index_generator - Missing 'recycled_indices' node",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15)"));

    foeIdIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(yaml_read_index_generator("", root, temp), foeYamlException,
                           Catch::Matchers::Equals("recycled_indices - Required node not found"));
}

TEST_CASE("yaml_read_index_generator - 'recycled_indices' with non-number",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [eight, 4, 10])"));

    foeIdIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator("", root, temp), foeYamlException,
        Catch::Matchers::Equals(
            "recycled_indices - Could not parse node as 'uint32_t' with value of: eight"));
}