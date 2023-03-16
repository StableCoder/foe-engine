// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/id.h>
#include <foe/ecs/id_to_string.hpp>

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
        CHECK(foeIdGroupToString(foeIdValueToGroup(0x4)) == "0x4");
        CHECK(foeIdIndexToString(0x404) == "0x0000404");
    }
    SECTION("Group 0xF") {
        CHECK(foeIdToString(foeIdCreate(foeIdValueToGroup(0xF), 0x404)) == "0xF0000404");
        CHECK(foeIdGroupToString(foeIdValueToGroup(0xF)) == "0xF");
        CHECK(foeIdIndexToString(0x404) == "0x0000404");
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