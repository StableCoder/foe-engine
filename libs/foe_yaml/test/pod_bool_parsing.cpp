// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

TEST_CASE("Reading of 'REQUIRED' bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: true
)"));

        bool testVal = false;
        REQUIRE_NOTHROW(yaml_read_required<bool>("topology", root, testVal));
        REQUIRE(testVal);

        SECTION("Reading from the passed-in node") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_read_required<bool>("", root["topology"], testVal));
            REQUIRE(testVal);
        }
    }
    SECTION("Node does not exist and fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: true
)"));

        bool testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<bool>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Required node not found to parse as 'bool'"));
    }
    SECTION("Node with bad data fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: truthy
)"));

        bool testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<bool>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'bool'"));
    }
}

TEST_CASE("Reading of 'OPTIONAL' bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Node exists and succeeds") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: true
)"));

        bool testVal = false;
        REQUIRE_NOTHROW(yaml_read_optional<bool>("topology", root, testVal));
        REQUIRE(testVal);
    }
    SECTION("Node does not exist and returns the given default value") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topLOL: true
)"));

        bool testVal = false;
        REQUIRE_NOTHROW(yaml_read_optional<bool>("topology", root, testVal));
        REQUIRE_FALSE(testVal);
        testVal = true;
        REQUIRE_NOTHROW(yaml_read_optional<bool>("topology", root, testVal));
        REQUIRE(testVal);
    }
    SECTION("Node with bad data fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: truthy
)"));

        bool testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_optional<bool>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Equals("topology - Could not parse Map-type node as 'bool'"));
    }
}

TEST_CASE("Writing of 'REQUIRED' bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully") {
        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_required("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE_FALSE(root["topology"].as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_required("topology", testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE(root["topology"].as<bool>());
        }
    }

    SECTION("Writes to same-node successfully") {
        root["topology"] = 10;
        auto node = root["topology"];

        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_required("", testVal, node));

            REQUIRE(node);
            REQUIRE_FALSE(node.as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_required("", testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<bool>());
        }
    }
}

TEST_CASE("Writing of 'OPTIONAL' bool YAML nodes", "[foe][yaml]") {
    YAML::Node root;

    SECTION("Writes to sub-node successfully IF not the same as default value") {
        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_optional("topology", true, testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE_FALSE(root["topology"].as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_optional("topology", false, testVal, root));

            REQUIRE(root["topology"]);
            REQUIRE(root["topology"].as<bool>());
        }
    }

    SECTION("Writes to same-node successfully IF not the same as default value") {
        root["topology"] = 10;
        auto node = root["topology"];

        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_optional("", true, testVal, node));

            REQUIRE(node);
            REQUIRE_FALSE(node.as<bool>());
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_optional("", false, testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<bool>());
        }
    }

    SECTION("Doesn't write to sub-node IF same as default value") {
        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_optional("topology", false, testVal, root));

            REQUIRE_FALSE(root["topology"]);
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_optional("topology", true, testVal, root));

            REQUIRE_FALSE(root["topology"]);
        }
    }

    SECTION("Doesn't write to same-node IF same as default value") {
        root["topology"] = 10;
        auto node = root["topology"];

        SECTION("'false' value") {
            bool testVal = false;
            REQUIRE_NOTHROW(yaml_write_optional("", false, testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<int>() == 10);
        }
        SECTION("'true' value") {
            bool testVal = true;
            REQUIRE_NOTHROW(yaml_write_optional("", true, testVal, node));

            REQUIRE(node);
            REQUIRE(node.as<int>() == 10);
        }
    }
}