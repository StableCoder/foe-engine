// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

TEST_CASE("Reading bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: true
)"));

        bool testVal = false;
        REQUIRE_NOTHROW(yaml_read_bool("topology", root, testVal));
        REQUIRE(testVal);

        SECTION("Reading from the passed-in node") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_read_bool("", root["topology"], testVal));
            REQUIRE(testVal);
        }
    }
    SECTION("Node does not exist and returns false") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: true
)"));

        bool testVal;
        REQUIRE_FALSE(yaml_read_bool("topology", root, testVal));
    }
    SECTION("Node with bad data fails with exception") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: truthy
)"));

        bool testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_bool("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'bool'"));
    }
}

TEST_CASE("Writing bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully") {
        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_bool("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE_FALSE(root["topology"].as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_bool("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE(root["topology"].as<bool>());
        }
    }

    SECTION("Writes to same-node successfully") {
        root["topology"] = 10;
        auto node = root["topology"];

        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_bool("", testVal, node));

            REQUIRE(node);
            REQUIRE_FALSE(node.as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_bool("", testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<bool>());
        }
    }
}
