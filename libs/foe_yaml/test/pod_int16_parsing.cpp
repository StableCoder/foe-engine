// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <cstdint>

TEST_CASE("Reading of 'REQUIRED' int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 111
)"));

        int16_t testVal = 0;
        REQUIRE_NOTHROW(yaml_read_required<int16_t>("topology", root, testVal));
        REQUIRE(testVal == 111);

        SECTION("Reading from the passed-in node") {
            int16_t testVal = 0;
            REQUIRE_NOTHROW(yaml_read_required<int16_t>("", root["topology"], testVal));
            REQUIRE(testVal == 111);
        }
    }
    SECTION("Node does not exist and fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: 1
)"));

        int16_t testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<int16_t>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Required node not found to parse as 'int16_t'"));
    }
    SECTION("Node with bad data fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 1a
)"));

        int16_t testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<int16_t>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'int16_t'"));
    }
}

TEST_CASE("Reading of 'OPTIONAL' int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 3
)"));

        int16_t testVal = 0;
        yaml_read_optional<int16_t>("topology", root, testVal);
        REQUIRE_NOTHROW(yaml_read_optional<int16_t>("topology", root, testVal));
        REQUIRE(testVal == 3);
    }
    SECTION("Node does not exist and returns the given default value") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: 2
)"));

        int16_t testVal = 0;
        REQUIRE_NOTHROW(yaml_read_optional<int16_t>("topology", root, testVal));
        REQUIRE(testVal == 0);
        testVal = 109;
        REQUIRE_NOTHROW(yaml_read_optional<int16_t>("topology", root, testVal));
        REQUIRE(testVal == 109);
    }
    SECTION("Node with bad data fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: truthy
)"));

        int16_t testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_optional<int16_t>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'int16_t'"));
    }
}

TEST_CASE("Writing of 'REQUIRED' int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully") {
        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_required("topology", testVal, root));

        REQUIRE(root["topology"]);
        REQUIRE(root["topology"].as<int16_t>() == 101);
    }

    SECTION("Writes to same-node successfully") {
        root["topology"] = 10;
        auto node = root["topology"];

        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_required("", testVal, node));

        REQUIRE(node);
        REQUIRE(node.as<int16_t>() == 101);
    }
}

TEST_CASE("Writing of 'OPTIONAL' int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully IF not the same as default value") {
        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_optional("topology", static_cast<int16_t>(100), testVal, root));

        REQUIRE(root["topology"]);
        REQUIRE(root["topology"].as<int16_t>() == 101);
    }

    SECTION("Writes to same-node successfully IF not the same as default value") {
        root["topology"] = 10;
        auto node = root["topology"];

        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_optional("", static_cast<int16_t>(100), testVal, node));

        REQUIRE(node);
        REQUIRE(node.as<int16_t>() == 101);
    }

    SECTION("Doesn't write to sub-node IF same as default value") {
        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_optional("topology", static_cast<int16_t>(101), testVal, root));

        REQUIRE_FALSE(root["topology"]);
    }

    SECTION("Doesn't write to same-node IF same as default value") {
        root["topology"] = 10;
        auto node = root["topology"];

        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_optional("", static_cast<int16_t>(101), testVal, node));

        REQUIRE(node);
        REQUIRE(node.as<int>() == 10);
    }
}