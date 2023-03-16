// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/component_pool.h>

#include <algorithm>
#include <array>
#include <atomic>

TEST_CASE("ComponentPool - Initially there are no associated entity lists") {
    foeEcsComponentPool testPool{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(std::atomic<int> *), nullptr, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    CHECK(foeEcsComponentPoolEntityListSize(testPool) == 0);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Adding and removing single entity list") {
    foeEcsComponentPool testPool{FOE_NULL_HANDLE};
    foeEcsEntityList testList{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(std::atomic<int> *), nullptr, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    result = foeEcsCreateEntityList(&testList);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testList != FOE_NULL_HANDLE);

    // Adding the list
    result = foeEcsComponentPoolAddEntityList(testPool, testList);
    REQUIRE(result.value == FOE_SUCCESS);

    // Check the list is accessible from the pool
    CHECK(foeEcsComponentPoolEntityListSize(testPool) == 1);
    CHECK(*foeEcsComponentPoolEntityLists(testPool) == testList);

    // Removing the list
    foeEcsComponentPoolRemoveEntityList(testPool, testList);

    // Check list is no longer accessible
    CHECK(foeEcsComponentPoolEntityListSize(testPool) == 0);

    foeEcsDestroyEntityList(testList);
    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Adding and removing multiple entity lists") {
    foeEcsComponentPool testPool{FOE_NULL_HANDLE};
    std::array<foeEcsEntityList, 12> testLists = {};
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(std::atomic<int> *), nullptr, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    for (size_t i = 0; i < testLists.size(); ++i) {
        result = foeEcsCreateEntityList(&testLists[i]);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(testLists[i] != FOE_NULL_HANDLE);
    }

    // Adding lists in a split fashion
    for (size_t i = testLists.size() / 2; i < testLists.size(); ++i) {
        result = foeEcsComponentPoolAddEntityList(testPool, testLists[i]);
        REQUIRE(result.value == FOE_SUCCESS);
    }
    CHECK(foeEcsComponentPoolEntityListSize(testPool) == testLists.size() / 2);
    CHECK(*foeEcsComponentPoolEntityLists(testPool) != nullptr);

    for (size_t i = 0; i < testLists.size() / 2; ++i) {
        result = foeEcsComponentPoolAddEntityList(testPool, testLists[i]);
        REQUIRE(result.value == FOE_SUCCESS);
    }
    CHECK(foeEcsComponentPoolEntityListSize(testPool) == testLists.size());
    CHECK(*foeEcsComponentPoolEntityLists(testPool) != nullptr);

    { // Check all lists are accessible from the pool
        foeEcsEntityList const *pLists = foeEcsComponentPoolEntityLists(testPool);
        foeEcsEntityList const *const pEndLists =
            pLists + foeEcsComponentPoolEntityListSize(testPool);

        for (size_t i = 0; i < testLists.size(); ++i) {
            foeEcsEntityList const *pList = std::find(pLists, pEndLists, testLists[i]);

            CHECK(pList != pEndLists);
        }
    }

    // Remove every other list
    for (size_t i = 0; i < testLists.size(); i += 2) {
        foeEcsComponentPoolRemoveEntityList(testPool, testLists[i]);
    }
    CHECK(foeEcsComponentPoolEntityListSize(testPool) == testLists.size() / 2);

    { // Check that the removed lists are not accessible, kept ones are
        foeEcsEntityList const *pLists = foeEcsComponentPoolEntityLists(testPool);
        foeEcsEntityList const *const pEndLists =
            pLists + foeEcsComponentPoolEntityListSize(testPool);

        for (size_t i = 0; i < testLists.size(); i += 2) {
            // Removed
            foeEcsEntityList const *pList = std::find(pLists, pEndLists, testLists[i]);
            CHECK(pList == pEndLists);

            // Remaining
            pList = std::find(pLists, pEndLists, testLists[i + 1]);
            CHECK(pList != pEndLists);
        }
    }

    // Cleanup
    for (size_t i = 0; i < testLists.size(); ++i) {
        foeEcsDestroyEntityList(testLists[i]);
    }
    foeEcsDestroyComponentPool(testPool);
}