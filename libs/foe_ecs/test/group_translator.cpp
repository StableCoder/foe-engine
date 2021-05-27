/*
    Copyright (C) 2021 George Cave.

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
#include <foe/ecs/group_translator.hpp>

TEST_CASE("GroupTranslator - Creation", "[foe][ecs]") {
    foeIdGroupTranslator test;
    std::vector<foeIdGroupValueNameSet> sources;
    std::vector<foeIdGroupValueNameSet> destinations;

    SECTION("Generating with no sources/destinations succeeds") {
        REQUIRE_FALSE(foeIdCreateTranslator(sources, destinations, &test));
    }

    SECTION("Generating with no sources but with destinations succeeds") {
        destinations = {
            {0, "0"},
            {1, "1"},
        };
        REQUIRE_FALSE(foeIdCreateTranslator(sources, destinations, &test));
    }

    SECTION("Generating with sources and matching destinations succeeds") {
        sources = {
            {1, "1"},
            {0, "0"},
        };
        destinations = {
            {3, "0"},
            {4, "1"},
        };

        REQUIRE_FALSE(foeIdCreateTranslator(sources, destinations, &test));
    }

    SECTION("Generating with sources with missing destinations fails") {
        sources = {
            {1, "1"},
            {0, "0"},
        };
        destinations = {
            {0, "0"},
        };
        REQUIRE(foeIdCreateTranslator(sources, destinations, &test));
    }
}

TEST_CASE("GroupTranslator - Translating Values", "[foe][ecs]") {
    foeIdGroupTranslator test;
    std::vector<foeIdGroupValueNameSet> sources = {
        {1, "1"},
        {0, "0"},
    };
    std::vector<foeIdGroupValueNameSet> destinations = {
        {3, "0"},
        {4, "1"},
        {15, "15"},
    };

    REQUIRE_FALSE(foeIdCreateTranslator(sources, destinations, &test));

    SECTION("Valid source values return valid groups") {
        REQUIRE(foeIdTranslateGroupValue(&test, 0) == foeIdValueToGroup(3));
        REQUIRE(foeIdTranslateGroupValue(&test, 1) == foeIdValueToGroup(4));
    }
    SECTION("Invalid source values return FOE_INVALID_ID") {
        REQUIRE(foeIdTranslateGroupValue(&test, 14) == FOE_INVALID_ID);
        REQUIRE(foeIdTranslateGroupValue(&test, 15) == FOE_INVALID_ID);
    }
}