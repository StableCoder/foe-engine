// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/component_pool.h>

#include <atomic>

TEST_CASE("ComponentPool - Check destructors called when component data is freed") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    std::atomic<int> testDestructorCallCount = 0;
    std::atomic<int> *const pAtomicCounter = &testDestructorCallCount;
    foeResultSet result;

    auto destructorCall = [](void *pData) {
        std::atomic<int> *const pAtomicCounter = *((std::atomic<int> **)pData);
        *pAtomicCounter += 1;
    };

    result = foeEcsCreateComponentPool(0, 1, sizeof(std::atomic<int> *), destructorCall, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    SECTION("Called when insert data is skipped (due to duplicates)") {
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 0);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 1);

        foeEcsDestroyComponentPool(testPool);
    }

    SECTION("Called when insert data is skipped (already inserted)") {
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 0);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 1);

        foeEcsDestroyComponentPool(testPool);
    }

    SECTION("Called when data is removed") {
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolRemove(testPool, foeEntityID(16));
        REQUIRE(result.value == FOE_SUCCESS);

        // Initial removal, the data is shifted tot he 'removed' pool
        CHECK(testDestructorCallCount == 0);
        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        // Second call data is actually discarded
        CHECK(testDestructorCallCount == 0);
        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 1);

        foeEcsDestroyComponentPool(testPool);
    }

    SECTION("Called when pool is destroyed on data in removed, regular and awaiting insertion") {
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolRemove(testPool, foeEntityID(16));
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolMaintenance(testPool);
        REQUIRE(result.value == FOE_SUCCESS);

        result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), (void *)&pAtomicCounter);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(testDestructorCallCount == 0);

        foeEcsDestroyComponentPool(testPool);

        CHECK(testDestructorCallCount == 3);
    }
}

TEST_CASE("ComponentPool - Maintenance with no changes does nothing") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(foeEcsComponentPoolCapacity(testPool) == 0);
    CHECK(foeEcsComponentPoolSize(testPool) == 0);
    CHECK(foeEcsComponentPoolInserted(testPool) == 0);
    CHECK(foeEcsComponentPoolRemoved(testPool) == 0);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Test initial capacity") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(1024, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 0);

    int val = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 1024);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Test reserving capacity") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 0);

    int val = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    foeEcsComponentPoolReserve(testPool, 1024);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 1024);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Test capacity expansion rate") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1024, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 0);

    int val = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 1024);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Test changing capacity expansion rate") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 0);

    int val = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    foeEcsComponentPoolExpansionRate(testPool, 1024);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolCapacity(testPool) == 1024);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Test setting insertion capacity") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    CHECK(foeEcsComponentPoolInsertCapacity(testPool) == 0);

    int val = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(foeEcsComponentPoolInsertCapacity(testPool) < 128);

    foeEcsComponentPoolReserveInsertCapacity(testPool, 8192);

    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &val);
    REQUIRE(result.value == FOE_SUCCESS);

    CHECK(foeEcsComponentPoolInsertCapacity(testPool) == 8192);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Single insertion") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    foeEntityID testID = 256;
    int testValue = 128;
    result = foeEcsComponentPoolInsert(testPool, testID, &testValue);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    SECTION("Entity has been inserted during maintenance, exists, and is accessible") {
        REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
        REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
        REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);
        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);

        REQUIRE(*pIDs == testID);
        REQUIRE(*pData == testValue);

        // Check the inserted offsets into the data is accurate
        size_t const *pInsertedOffsets = foeEcsComponentPoolInsertedOffsetPtr(testPool);
        REQUIRE(pInsertedOffsets != nullptr);

        REQUIRE(*(pIDs + (*pInsertedOffsets)) == testID);
        REQUIRE(*(pData + (*pInsertedOffsets)) == testValue);
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Multiple reversed insertion") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(uint32_t), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    constexpr int cTestNum = 3;

    for (int i = cTestNum - 1; i >= 0; --i) {
        int val = i;
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(i + 1), &val);
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    SECTION("Entity has been inserted during maintenance, exists, and is accessible") {
        REQUIRE(foeEcsComponentPoolSize(testPool) == cTestNum);
        REQUIRE(foeEcsComponentPoolInserted(testPool) == cTestNum);
        REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);

        // Regular data
        for (int i = 0; i < cTestNum; ++i) {
            CHECK(pIDs[i] == foeEntityID(i + 1));
            CHECK(pData[i] == i);
        }

        // Inserted Offsets
        size_t const *pInsertedOffsets = foeEcsComponentPoolInsertedOffsetPtr(testPool);

        REQUIRE(pInsertedOffsets != nullptr);

        for (int i = 0; i < cTestNum; ++i) {
            CHECK(pIDs[pInsertedOffsets[i]] == foeEntityID(i + 1));
            CHECK(pData[pInsertedOffsets[i]] == i);
        }
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Staggered insertion") {
    // In this case, we'll first insert a group of even numbers in a pass, then in the next pass,
    // the interleaving set of odd numbers, ensuring that they interleave and sort out
    // correctly.
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    constexpr int cTestNum = 20;

    for (int i = cTestNum - 2; i >= 0; i -= 2) {
        int val = i;
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(i + 1), &val);
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 10);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 10);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    SECTION("even items should exist, and in ascending order") {
        SECTION("Pool") {
            foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
            int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

            REQUIRE(pIDs != nullptr);
            REQUIRE(pData != nullptr);

            for (int i = 0; i < cTestNum; i += 2, ++pIDs, ++pData) {
                CHECK(*pIDs == foeEntityID(i + 1));
                CHECK(*pData == i);
            }
        }

        SECTION("Inserted Offsets") {
            foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
            int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);
            size_t const *pInsertedOffsets = foeEcsComponentPoolInsertedOffsetPtr(testPool);

            REQUIRE(pIDs != nullptr);
            REQUIRE(pData != nullptr);
            REQUIRE(pInsertedOffsets != nullptr);

            for (int i = 0; i < cTestNum; i += 2, ++pInsertedOffsets) {
                CHECK(pIDs[*pInsertedOffsets] == foeEntityID(i + 1));
                CHECK(pData[*pInsertedOffsets] == i);
            }
        }
    }

    // Inserting the odd numbers
    for (int i = cTestNum - 1; i >= 0; i -= 2) {
        int val = i;
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(i + 1), &val);
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 20);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 10);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    SECTION("all numbers exist, in correct order") {
        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);

        for (int i = 0; i < cTestNum; ++i, ++pIDs, ++pData) {
            CHECK(*pIDs == foeEntityID(i + 1));
            CHECK(*pData == i);
        }
    }
    SECTION("odd numbered insertions are the only ones in the insertion list, in order") {
        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);
        size_t const *pInsertedOffsets = foeEcsComponentPoolInsertedOffsetPtr(testPool);

        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);
        REQUIRE(pInsertedOffsets != nullptr);

        for (int i = 1; i < cTestNum; i += 2, ++pInsertedOffsets) {
            CHECK(pIDs[*pInsertedOffsets] == foeEntityID(i + 1));
            CHECK(pData[*pInsertedOffsets] == i);
        }
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE(
    "ComponentPool - Attempting to insert multiple of the same EntityID fails, only the *last* one "
    "is inserted") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 2;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 3;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
    int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

    REQUIRE(pIDs != nullptr);
    REQUIRE(pData != nullptr);

    CHECK(*pIDs == foeEntityID(16));
    CHECK(*pData == temp);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Attempting to add same entity in a different pass fails, original stays "
          "intact") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 1;
    result = foeEcsComponentPoolInsert(testPool, uint32_t(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    temp = 2;
    result = foeEcsComponentPoolInsert(testPool, uint32_t(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
    int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

    REQUIRE(pIDs != nullptr);
    REQUIRE(pData != nullptr);

    CHECK(*pIDs == foeEntityID(16));
    CHECK(*pData == 1);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Attempting to add same entity in a different pass fails, original stays "
          "intact, other content inserts") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 1;
    result = foeEcsComponentPoolInsert(testPool, uint32_t(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    temp = 2;
    result = foeEcsComponentPoolInsert(testPool, uint32_t(16), &temp);
    REQUIRE(result.value == FOE_SUCCESS);
    temp = 2;
    result = foeEcsComponentPoolInsert(testPool, uint32_t(17), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 2);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
    int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

    REQUIRE(pIDs != nullptr);
    REQUIRE(pData != nullptr);

    CHECK(pIDs[0] == foeEntityID(16));
    CHECK(pData[0] == 1);
    CHECK(pIDs[1] == foeEntityID(17));
    CHECK(pData[1] == 2);

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Single removal") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 128;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(256), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
    int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

    REQUIRE(pIDs != nullptr);
    REQUIRE(pData != nullptr);

    CHECK(*pIDs == foeEntityID(256));
    CHECK(*pData == temp);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 1);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 1);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    result = foeEcsComponentPoolRemove(testPool, foeEntityID(256));
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 0);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 1);

    SECTION("Entity has been removed during maintenance, no longer exists, and is accessible") {
        foeEntityID const *pRemovedIDs = foeEcsComponentPoolRemovedIdPtr(testPool);
        int const *pRemovedData = (int *)foeEcsComponentPoolRemovedDataPtr(testPool);

        REQUIRE(pRemovedIDs != nullptr);
        REQUIRE(pRemovedData != nullptr);

        CHECK(*pRemovedIDs == foeEntityID(256));
        CHECK(*pRemovedData == temp);
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Multiple removal") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int val = i;
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(i + 1), &val);
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 20);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 20);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    for (int i = cTestNum; i >= 0; --i) {
        result = foeEcsComponentPoolRemove(testPool, foeEntityID(i + 1));
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    SECTION("Entities have been removed during maintenance, no longer exists, and is accessible") {
        REQUIRE(foeEcsComponentPoolSize(testPool) == 0);
        REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
        REQUIRE(foeEcsComponentPoolRemoved(testPool) == 20);

        foeEntityID const *pRemovedIDs = foeEcsComponentPoolRemovedIdPtr(testPool);
        int const *pRemovedData = (int *)foeEcsComponentPoolRemovedDataPtr(testPool);

        REQUIRE(pRemovedIDs != nullptr);
        REQUIRE(pRemovedData != nullptr);

        for (int i = 0; i < cTestNum; ++i, ++pRemovedIDs, ++pRemovedData) {
            CHECK(*pRemovedIDs == foeEntityID(i + 1));
            CHECK(*pRemovedData == i);
        }
    }

    foeEcsDestroyComponentPool(testPool);
}
TEST_CASE("ComponentPool - Staggered removal") {
    // All items are added, then just the odd ones are removed, making sure that removals are
    // correct.
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int val = i;
        result = foeEcsComponentPoolInsert(testPool, foeEntityID(i + 1), &val);
        REQUIRE(result.value == FOE_SUCCESS);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == cTestNum);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == cTestNum);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    std::vector<int> removedIDs;
    for (int i = cTestNum - 1; i >= 0; i -= 2) {
        result = foeEcsComponentPoolRemove(testPool, foeEntityID(i));
        REQUIRE(result.value == FOE_SUCCESS);
        removedIDs.emplace_back(i);
    }

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == cTestNum / 2);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == cTestNum / 2);

    SECTION("Entities that should remain do so, and in order") {
        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);

        for (int i = 2; i < cTestNum; i += 2, ++pIDs, ++pData) {
            CHECK(*pIDs == foeEntityID(i));
            CHECK(*pData == i - 1);
        }
    }
    SECTION("Entities have been removed during maintenance, no longer exists, and is accessible") {
        foeEntityID const *pRemovedIDs = foeEcsComponentPoolRemovedIdPtr(testPool);
        int const *pRemovedData = (int *)foeEcsComponentPoolRemovedDataPtr(testPool);

        REQUIRE(pRemovedIDs != nullptr);
        REQUIRE(pRemovedData != nullptr);

        for (int i = 1; i < cTestNum; i += 2, ++pRemovedIDs, ++pRemovedData) {
            CHECK(*pRemovedIDs == foeEntityID(i));
            CHECK(*pRemovedData == i - 1);
        }
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE(
    "ComponentPool - Attempting to remove an item multiple times doesn't have undesired effects") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 10;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(10), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 12;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(12), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 8;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(8), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 3);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 3);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    result = foeEcsComponentPoolRemove(testPool, foeEntityID(10));
    REQUIRE(result.value == FOE_SUCCESS);
    result = foeEcsComponentPoolRemove(testPool, foeEntityID(10));
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 2);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 1);

    SECTION("Remaining items") {
        foeEntityID const *pIDs = foeEcsComponentPoolIdPtr(testPool);
        int const *pData = (int *)foeEcsComponentPoolDataPtr(testPool);

        REQUIRE(pIDs != nullptr);
        REQUIRE(pData != nullptr);

        CHECK(pIDs[0] == foeEntityID(8));
        CHECK(pData[0] == 8);
        CHECK(pIDs[1] == foeEntityID(12));
        CHECK(pData[1] == 12);
    }
    SECTION("Removed item") {
        foeEntityID const *pRemovedIDs = foeEcsComponentPoolRemovedIdPtr(testPool);
        int const *pRemovedData = (int *)foeEcsComponentPoolRemovedDataPtr(testPool);

        REQUIRE(pRemovedIDs != nullptr);
        REQUIRE(pRemovedData != nullptr);

        CHECK(pRemovedIDs[0] == foeEntityID(10));
        CHECK(pRemovedData[0] == 10);
    }

    foeEcsDestroyComponentPool(testPool);
}

TEST_CASE("ComponentPool - Attempting to remove items not in the pool has no undesired effects") {
    foeEcsComponentPool testPool = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeEcsCreateComponentPool(0, 1, sizeof(int), NULL, &testPool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(testPool != FOE_NULL_HANDLE);

    int temp = 1;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(1), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 3;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(3), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    temp = 5;
    result = foeEcsComponentPoolInsert(testPool, foeEntityID(5), &temp);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 3);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 3);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    result = foeEcsComponentPoolRemove(testPool, foeEntityID(0));
    REQUIRE(result.value == FOE_SUCCESS);
    result = foeEcsComponentPoolRemove(testPool, foeEntityID(2));
    REQUIRE(result.value == FOE_SUCCESS);
    result = foeEcsComponentPoolRemove(testPool, foeEntityID(4));
    REQUIRE(result.value == FOE_SUCCESS);
    result = foeEcsComponentPoolRemove(testPool, foeEntityID(6));
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeEcsComponentPoolMaintenance(testPool);
    REQUIRE(result.value == FOE_SUCCESS);

    REQUIRE(foeEcsComponentPoolSize(testPool) == 3);
    REQUIRE(foeEcsComponentPoolInserted(testPool) == 0);
    REQUIRE(foeEcsComponentPoolRemoved(testPool) == 0);

    foeEcsDestroyComponentPool(testPool);
}
