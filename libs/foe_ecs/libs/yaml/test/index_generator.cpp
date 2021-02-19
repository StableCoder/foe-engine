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
#include <foe/ecs/index_generator.hpp>
#include <foe/ecs/yaml/index_generator.hpp>
#include <foe/yaml/exception.hpp>

TEST_CASE("yaml_read_index_generator - Success Case", "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [8, 4, 10])"));

    foeEcsIndexGenerator testGenerator{"", 0};
    REQUIRE_NOTHROW(yaml_read_index_generator(root, testGenerator));

    CHECK(testGenerator.generate() == 8);
    CHECK(testGenerator.generate() == 4);
    CHECK(testGenerator.generate() == 10);
    CHECK(testGenerator.generate() == 15);
    CHECK(testGenerator.generate() == 16);
}

TEST_CASE("yaml_write_index_generator", "[foe][ecs][yaml][IndexGenerator]") {
    foeEcsIndexGenerator indexGen{"", 0};

    for (int i = 0; i < 15; ++i) {
        indexGen.generate();
    }
    indexGen.free(8);
    indexGen.free(4);
    indexGen.free(10);

    YAML::Node testNode;
    REQUIRE_NOTHROW(testNode = yaml_write_index_generator(indexGen));

    YAML::Emitter emitter;
    emitter << testNode;

    std::string show = emitter.c_str();

    foeEcsIndexGenerator testGenerator{"", 0};
    REQUIRE_NOTHROW(yaml_read_index_generator(testNode, testGenerator));

    CHECK(testGenerator.generate() == 8);
    CHECK(testGenerator.generate() == 4);
    CHECK(testGenerator.generate() == 10);
    CHECK(testGenerator.generate() == 15);
    CHECK(testGenerator.generate() == 16);
}

TEST_CASE("yaml_read_index_generator - Missing 'next_free_index' node",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(recycled_indices: [8, 4, 10])"));

    foeEcsIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator(root, temp), foeYamlException,
        Catch::Matchers::Equals(
            "yaml_read_index_generator - Could not find required 'next_free_index' node"));
}

TEST_CASE("yaml_read_index_generator - 'next_free_index' node not integer") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: fifteen
recycled_indices: [8, 4, 10])"));

    foeEcsIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator(root, temp), foeYamlException,
        Catch::Matchers::Equals("yaml_read_index_generator::next_free_index - Could not parse "
                                "value of 'fifteen' to foeIndexID"));
}

TEST_CASE("yaml_read_index_generator - Missing 'recycled_indices' node",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15)"));

    foeEcsIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator(root, temp), foeYamlException,
        Catch::Matchers::Equals(
            "yaml_read_index_generator - Could not find required 'recycled_indices' node"));
}

TEST_CASE("yaml_read_index_generator - 'recycled_indices' with non-number",
          "[foe][ecs][yaml][IndexGenerator]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [eight, 4, 10])"));

    foeEcsIndexGenerator temp{"", 0};
    REQUIRE_THROWS_MATCHES(
        yaml_read_index_generator(root, temp), foeYamlException,
        Catch::Matchers::Equals("yaml_read_index_generator::recycled_indices - Could not parse "
                                "value of 'eight' as a foeIndexID"));
}