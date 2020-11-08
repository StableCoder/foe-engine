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
#include <foe/yaml/pod_parsing.hpp>

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
            Catch::Matchers::EndsWith(" - Required node not found to parse as 'int16_t'"));
    }
    SECTION("Node with bad data fails") {
        REQUIRE_NOTHROW(root = YAML::Load(R"(topology: 1a
)"));

        int16_t testVal;
        REQUIRE_THROWS_MATCHES(
            yaml_read_required<int16_t>("topology", root, testVal), foeYamlException,
            Catch::Matchers::Contains(" - Could not parse node as 'int16_t' with value of: "));
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
            Catch::Matchers::Contains(" - Could not parse node as 'int16_t' with value of: "));
    }
}