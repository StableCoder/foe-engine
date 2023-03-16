// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/entity_list.h>
#include <foe/ecs/result.h>

#include <array>

TEST_CASE("EntityList - Initial State") {
    foeEcsEntityList test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateEntityList(&test);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    CHECK(foeEcsEntityListSize(test) == 0);

    foeEcsDestroyEntityList(test);
}

TEST_CASE("EntityList - Resetting with data") {
    foeEcsEntityList test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateEntityList(&test);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("No data") {
        result = foeEcsResetEntityList(test, 0, nullptr, nullptr);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(foeEcsEntityListSize(test) == 0);
    }

    SECTION("Single list of data") {
        std::array<foeEntityID, 4> testData = {1024, 2048, 4096, 8192};

        std::array<foeEntityID *, 1> testDataLists = {testData.data()};
        std::array<uint32_t, 1> testDataListSizes = {testData.size()};

        result = foeEcsResetEntityList(test, testDataListSizes.size(), testDataListSizes.data(),
                                       testDataLists.data());
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(foeEcsEntityListSize(test) == 4);

        foeEntityID const *pEntityList = foeEcsEntityListPtr(test);
        REQUIRE(pEntityList != nullptr);

        CHECK(pEntityList[0] == testData[0]);
        CHECK(pEntityList[1] == testData[1]);
        CHECK(pEntityList[2] == testData[2]);
        CHECK(pEntityList[3] == testData[3]);

        SECTION("Further reset with no data") {
            result = foeEcsResetEntityList(test, 0, nullptr, nullptr);
            REQUIRE(result.value == FOE_SUCCESS);

            CHECK(foeEcsEntityListSize(test) == 0);
        }
    }

    SECTION("Multiple lists of data") {
        std::array<foeEntityID, 4> testData = {1024, 2048, 4096, 8192};
        std::array<foeEntityID, 4> testData2 = {9000, 9001, 9002, 9003};

        std::array<foeEntityID *, 2> testDataLists = {testData.data(), testData2.data()};
        std::array<uint32_t, 2> testDataListSizes = {testData.size(), testData2.size()};

        result = foeEcsResetEntityList(test, testDataListSizes.size(), testDataListSizes.data(),
                                       testDataLists.data());
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(foeEcsEntityListSize(test) == 8);

        foeEntityID const *pEntityList = foeEcsEntityListPtr(test);
        REQUIRE(pEntityList != nullptr);

        CHECK(pEntityList[0] == testData[0]);
        CHECK(pEntityList[1] == testData[1]);
        CHECK(pEntityList[2] == testData[2]);
        CHECK(pEntityList[3] == testData[3]);

        CHECK(pEntityList[4] == testData2[0]);
        CHECK(pEntityList[5] == testData2[1]);
        CHECK(pEntityList[6] == testData2[2]);
        CHECK(pEntityList[7] == testData2[3]);

        SECTION("Further reset with no data") {
            result = foeEcsResetEntityList(test, 0, nullptr, nullptr);
            REQUIRE(result.value == FOE_SUCCESS);

            CHECK(foeEcsEntityListSize(test) == 0);
        }
    }

    foeEcsDestroyEntityList(test);
}