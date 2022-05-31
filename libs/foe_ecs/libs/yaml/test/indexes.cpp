/*
    Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca

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
#include <foe/ecs/error_code.h>
#include <foe/ecs/indexes.h>
#include <foe/ecs/yaml/indexes.hpp>
#include <foe/yaml/exception.hpp>

TEST_CASE("yaml_read_indexes", "[foe][ecs][yaml][foeEcsIndexes]") {
    YAML::Node root;
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    SECTION("Local Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [8, 4, 10])"));
        REQUIRE_NOTHROW(yaml_read_indexes("", root, testIndexes));
    }
    SECTION("Sub Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(subNode:
  next_free_index: 15
  recycled_indices: [8, 4, 10])"));
        REQUIRE_NOTHROW(yaml_read_indexes("subNode", root, testIndexes));
    }

    foeId id = FOE_INVALID_ID;

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 8);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 4);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 10);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 15);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 16);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("yaml_write_indexes", "[foe][ecs][yaml][foeEcsIndexes]") {
    foeEcsIndexes indexes{FOE_NULL_HANDLE};
    foeId id = FOE_INVALID_ID;

    REQUIRE(foeEcsCreateIndexes(0, &indexes).value == FOE_ECS_SUCCESS);
    CHECK(indexes != FOE_NULL_HANDLE);

    for (int i = 0; i < 15; ++i) {
        CHECK(foeEcsGenerateID(indexes, &id).value == FOE_ECS_SUCCESS);
    }
    CHECK(id == 15);

    CHECK(foeEcsFreeID(indexes, 8).value == FOE_ECS_SUCCESS);
    CHECK(foeEcsFreeID(indexes, 4).value == FOE_ECS_SUCCESS);
    CHECK(foeEcsFreeID(indexes, 10).value == FOE_ECS_SUCCESS);

    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    SECTION("Local Node") {
        YAML::Node testNode;
        REQUIRE_NOTHROW(yaml_write_indexes("", indexes, testNode));
        REQUIRE_NOTHROW(yaml_read_indexes("", testNode, testIndexes));
    }
    SECTION("Sub Node") {
        YAML::Node testNode;
        REQUIRE_NOTHROW(yaml_write_indexes("subNode", indexes, testNode));
        REQUIRE_NOTHROW(yaml_read_indexes("subNode", testNode, testIndexes));
    }

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 8);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 4);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 10);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 16);

    CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(id == 17);

    foeEcsDestroyIndexes(testIndexes);
    foeEcsDestroyIndexes(indexes);
}

TEST_CASE("yaml_read_indexes - Missing as subNode") {
    YAML::Node root;
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    REQUIRE_NOTHROW(root = YAML::Load(R"(dummy:
  recycled_indices: [8, 4, 10])"));
    REQUIRE_THROWS_MATCHES(yaml_read_indexes("subNode", root, testIndexes), foeYamlException,
                           Catch::Matchers::Equals("subNode - Required node to parse not found"));

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("yaml_read_indexes - Missing 'next_free_index' node", "[foe][ecs][yaml][foeEcsIndexes]") {
    YAML::Node root;
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    SECTION("Local Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(recycled_indices: [8, 4, 10])"));
        REQUIRE_THROWS_MATCHES(
            yaml_read_indexes("", root, testIndexes), foeYamlException,
            Catch::Matchers::Equals(
                "next_free_index - Required node not found to parse as 'uint32_t'"));
    }
    SECTION("Sub Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(subNode:
  recycled_indices: [8, 4, 10])"));
        REQUIRE_THROWS_MATCHES(
            yaml_read_indexes("subNode", root, testIndexes), foeYamlException,
            Catch::Matchers::Equals(
                "subNode::next_free_index - Required node not found to parse as 'uint32_t'"));
    }

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("yaml_read_indexes - 'next_free_index' node not integer",
          "[foe][ecs][yaml][foeEcsIndexes]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: fifteen
recycled_indices: [8, 4, 10])"));

    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    REQUIRE_THROWS_MATCHES(
        yaml_read_indexes("", root, testIndexes), foeYamlException,
        Catch::Matchers::Equals("next_free_index - Could not parse Map-type node as 'uint32_t'"));

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("yaml_read_indexes - Missing 'recycled_indices' node",
          "[foe][ecs][yaml][foeEcsIndexes]") {
    YAML::Node root;
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    SECTION("Local Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15)"));
        REQUIRE_THROWS_MATCHES(
            yaml_read_indexes("", root, testIndexes), foeYamlException,
            Catch::Matchers::Equals("recycled_indices - Required node not found"));
    }
    SECTION("Sub Node") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(subNode:
  next_free_index: 15)"));
        REQUIRE_THROWS_MATCHES(
            yaml_read_indexes("subNode", root, testIndexes), foeYamlException,
            Catch::Matchers::Equals("subNode::recycled_indices - Required node not found"));
    }

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("yaml_read_indexes - 'recycled_indices' with non-number",
          "[foe][ecs][yaml][foeEcsIndexes]") {
    YAML::Node root;
    REQUIRE_NOTHROW(root = YAML::Load(R"(next_free_index: 15
recycled_indices: [eight, 4, 10])"));

    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    REQUIRE_THROWS_MATCHES(
        yaml_read_indexes("", root, testIndexes), foeYamlException,
        Catch::Matchers::Equals(
            "recycled_indices - Could not parse node as 'uint32_t' with value of: eight"));

    foeEcsDestroyIndexes(testIndexes);
}