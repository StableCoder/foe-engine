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
#include <foe/ecs/id.hpp>

TEST_CASE("foeID - Creating and converting IDs", "[foe][ecs]") {
    foeIdIndex index = 12345;
    foeIdGroup group = foeIdValueToGroup(12);
    foeId test;

    SECTION("Create Generic ID") {
        test = foeIdCreate(foeIdValueToGroup(0xF), 0xFF);

        REQUIRE(foeIdGroupToValue(foeIdGetGroup(test)) == 0xF);
        REQUIRE(foeIdGetIndex(test) == 0xFF);
    }

    SECTION("Create ") {
        test = foeIdCreate(group, index);

        REQUIRE(foeIdGetIndex(test) == index);
        REQUIRE(foeIdGetGroup(test) == group);
    }
}

TEST_CASE("foeID - Stringify functionality", "[foe][ecs]") {
    SECTION("Group 0x4") {
        CHECK(foeIdToString(foeIdCreate(foeIdValueToGroup(0x4), 0x404)) == "0x40000404");
        CHECK(foeIdToSplitString(foeIdCreate(foeIdValueToGroup(0x4), 0x404)) == "0x4-0x0000404");
    }
    SECTION("Group 0xF") {
        CHECK(foeIdToString(foeIdCreate(foeIdValueToGroup(0xF), 0x404)) == "0xF0000404");
        CHECK(foeIdToSplitString(foeIdCreate(foeIdValueToGroup(0xF), 0x404)) == "0xF-0x0000404");
    }
}

TEST_CASE("foeID - Testing Value Conversions", "[foe][ecs]") {
    SECTION("Group") {
        CHECK(foeIdValueToGroup(foeIdPersistentGroupValue) == foeIdPersistentGroup);
        CHECK(foeIdGroupToValue(foeIdPersistentGroup) == foeIdPersistentGroupValue);

        CHECK(foeIdValueToGroup(foeIdTemporaryGroupValue) == foeIdTemporaryGroup);
        CHECK(foeIdGroupToValue(foeIdTemporaryGroup) == foeIdTemporaryGroupValue);
    }

    SECTION("Index") {
        CHECK(foeIdValueToIndex(foeIdIndexMaxValue) == foeIdIndexMax);
        CHECK(foeIdIndexToValue(foeIdIndexMax) == foeIdIndexMaxValue);
    }
}