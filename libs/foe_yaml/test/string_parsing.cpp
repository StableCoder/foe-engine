// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

TEST_CASE("Reading string YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: star
)"));

        std::string testVal;
        REQUIRE_NOTHROW(yaml_read_string("topology", root, testVal));
        REQUIRE(testVal == "star");

        SECTION("Reading from the passed-in node") {
            std::string testVal;
            REQUIRE_NOTHROW(yaml_read_string("", root["topology"], testVal));
            REQUIRE(testVal == "star");
        }
    }
    SECTION("Node does not exist and returns false") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: true
)"));

        std::string testVal;
        REQUIRE_FALSE(yaml_read_string("topology", root, testVal));
        REQUIRE(testVal.empty());
    }
    SECTION("Map node with bad data fails with exception") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: [truthy]
)"));

        std::string testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_string("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'string'"));
        REQUIRE(testVal.empty());
    }
}

TEST_CASE("Writing string YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully") {
        SECTION("'star' value") {
            std::string testVal = "star";
            REQUIRE_NOTHROW(yaml_write_string("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE(root["topology"].as<std::string>() == "star");
        }
        SECTION("'web' value") {
            std::string testVal = "web";
            REQUIRE_NOTHROW(yaml_write_string("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE(root["topology"].as<std::string>() == "web");
        }
    }

    SECTION("Writes to same-node successfully") {
        root["topology"] = 10;
        auto node = root["topology"];

        SECTION("'star' value") {
            std::string testVal = "star";
            REQUIRE_NOTHROW(yaml_write_string("", testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<std::string>() == "star");
        }
        SECTION("'web' value") {
            std::string testVal = "web";
            REQUIRE_NOTHROW(yaml_write_string("", testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<std::string>() == "web");
        }
    }
}
