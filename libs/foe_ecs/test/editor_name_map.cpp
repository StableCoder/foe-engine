/*
    Copyright (C) 2021-2022 George Cave.

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
#include <foe/ecs/name_map.h>

#include <cstring>

TEST_CASE("EditorNameMap - Adding") {
    foeEcsNameMap testNameMap = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateNameMap(&testNameMap).value == FOE_ECS_SUCCESS);
    REQUIRE(testNameMap != FOE_NULL_HANDLE);

    SECTION("ID with empty name fails") {
        REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "").value == FOE_ECS_ERROR_EMPTY_NAME);
    }

    SECTION("Adding invalid entity fails") {
        REQUIRE(foeEcsNameMapAdd(testNameMap, FOE_INVALID_ID, "real").value ==
                FOE_ECS_ERROR_INVALID_ID);
    }

    SECTION("Adding two of same ID but different EditorNames fails") {
        REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "one").value == FOE_ECS_SUCCESS);
        REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "two").value == FOE_ECS_ERROR_ID_ALREADY_EXISTS);
    }

    SECTION("Adding two different ID but same EditorNames fails") {
        REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "one").value == FOE_ECS_SUCCESS);
        REQUIRE(foeEcsNameMapAdd(testNameMap, 2, "one").value == FOE_ECS_ERROR_NAME_ALREADY_EXISTS);
    }

    foeEcsDestroyNameMap(testNameMap);
}

TEST_CASE("EditorNameMap - Finding/Searching") {
    foeEcsNameMap testNameMap = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateNameMap(&testNameMap).value == FOE_ECS_SUCCESS);
    REQUIRE(testNameMap != FOE_NULL_HANDLE);

    REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "entity0").value == FOE_ECS_SUCCESS);

    SECTION("Finding existing set succeeds") {
        foeId id;
        REQUIRE(foeEcsNameMapFindID(testNameMap, "entity0", &id).value == FOE_ECS_SUCCESS);
        REQUIRE(id == 1);

        uint32_t strLength;
        char testStr[15];
        REQUIRE(foeEcsNameMapFindName(testNameMap, 1, &strLength, nullptr).value ==
                FOE_ECS_SUCCESS);
        CHECK(strLength == 8);

        SECTION("Trying with a smaller given string length returns FOE_ECS_INCOMPLETE") {
            strLength = 4;
            CHECK(foeEcsNameMapFindName(testNameMap, 1, &strLength, testStr).value ==
                  FOE_ECS_INCOMPLETE);
            CHECK(memcmp(testStr, "enti", 4) == 0);
        }

        strLength = 15;
        REQUIRE(foeEcsNameMapFindName(testNameMap, 1, &strLength, testStr).value ==
                FOE_ECS_SUCCESS);
        CHECK(strLength == 8);
        CHECK(memcmp(testStr, "entity0", 8) == 0);
    }

    SECTION("Finding non-existant set fails") {
        foeId id;
        REQUIRE(foeEcsNameMapFindID(testNameMap, "", &id).value == FOE_ECS_NO_MATCH);
        REQUIRE(foeEcsNameMapFindID(testNameMap, "entity1", &id).value == FOE_ECS_NO_MATCH);

        uint32_t strLength;
        REQUIRE(foeEcsNameMapFindName(testNameMap, 2, &strLength, nullptr).value ==
                FOE_ECS_NO_MATCH);
    }

    foeEcsDestroyNameMap(testNameMap);
}

TEST_CASE("EditorNameMap - Updating") {
    foeEcsNameMap testNameMap = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateNameMap(&testNameMap).value == FOE_ECS_SUCCESS);
    REQUIRE(testNameMap != FOE_NULL_HANDLE);

    REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "entity0").value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsNameMapAdd(testNameMap, 2, "entity1").value == FOE_ECS_SUCCESS);

    SECTION("Updating with unused names succeeds") {
        REQUIRE(foeEcsNameMapUpdate(testNameMap, 1, "entity2").value == FOE_ECS_SUCCESS);
    }

    SECTION("Updating set with already used name fails") {
        REQUIRE(foeEcsNameMapUpdate(testNameMap, 1, "entity1").value ==
                FOE_ECS_ERROR_NAME_ALREADY_EXISTS);
    }

    SECTION("Updating items not in the map fails") {
        REQUIRE(foeEcsNameMapUpdate(testNameMap, 3, "blah").value == FOE_ECS_ERROR_NO_MATCH);
    }

    SECTION("Updating with blank names fails") {
        REQUIRE(foeEcsNameMapUpdate(testNameMap, 1, "").value == FOE_ECS_ERROR_EMPTY_NAME);
    }

    foeEcsDestroyNameMap(testNameMap);
}

TEST_CASE("EditorNameMap - Removal") {
    foeEcsNameMap testNameMap = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateNameMap(&testNameMap).value == FOE_ECS_SUCCESS);
    REQUIRE(testNameMap != FOE_NULL_HANDLE);

    REQUIRE(foeEcsNameMapAdd(testNameMap, 1, "entity0").value == FOE_ECS_SUCCESS);

    SECTION("Removing set not in the map fails") {
        REQUIRE(foeEcsNameMapRemove(testNameMap, 3).value == FOE_ECS_NO_MATCH);
    }

    SECTION("Removing set in map succeeds") {
        REQUIRE(foeEcsNameMapRemove(testNameMap, 1).value == FOE_ECS_SUCCESS);
        SECTION("Attempting to remove set already removed fails") {
            CHECK(foeEcsNameMapRemove(testNameMap, 1).value == FOE_ECS_NO_MATCH);
        }
        SECTION("Adding removed set with real name succeeds") {
            CHECK(foeEcsNameMapAdd(testNameMap, 1, "entity123").value == FOE_ECS_SUCCESS);
        }
    }

    foeEcsDestroyNameMap(testNameMap);
}