// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include <cstdint>

TEST_CASE("Reading int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 111
)"));

        int16_t testVal = 0;
        REQUIRE(yaml_read_int16_t("topology", root, testVal));
        REQUIRE(testVal == 111);

        SECTION("Reading from the passed-in node") {
            int16_t testVal = 0;
            REQUIRE(yaml_read_int16_t("", root["topology"], testVal));
            REQUIRE(testVal == 111);
        }
    }
    SECTION("Node does not exist and returns false") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: 1
)"));

        int16_t testVal;
        REQUIRE_FALSE(yaml_read_int16_t("topology", root, testVal));
    }
    SECTION("Node with bad data fails with exception") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 1a
)"));

        int16_t testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_int16_t("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'int16_t'"));
    }
}

TEST_CASE("Writing int16_t YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully") {
        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_int16_t("topology", testVal, root));

        REQUIRE(root["topology"]);
        REQUIRE(root["topology"].as<int16_t>() == 101);
    }

    SECTION("Writes to same-node successfully") {
        root["topology"] = 10;
        auto node = root["topology"];

        int16_t testVal = 101;
        REQUIRE_NOTHROW(yaml_write_int16_t("", testVal, node));

        REQUIRE(node);
        REQUIRE(node.as<int16_t>() == 101);
    }
}
