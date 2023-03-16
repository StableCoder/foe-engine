// Copyright (C) 2021-2023 George Cave
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/indexes.h>
#include <foe/ecs/result.h>

#include <algorithm>
#include <array>
#include <thread>
#include <vector>

TEST_CASE("foeEcsIndexes - default state", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};
    foeIdIndex nextIndex;
    uint32_t recycledCount;

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recycledCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    CHECK(nextIndex == 1);
    CHECK(recycledCount == 0);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - plain reset", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    foeIdIndex nextIndex;
    uint32_t recycledCount;
    foeId id;

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recycledCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    CHECK(nextIndex == 1);
    CHECK(recycledCount == 0);

    REQUIRE(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    CHECK(foeEcsFreeID(testIndexes, id).value == FOE_ECS_SUCCESS);

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recycledCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    CHECK(nextIndex == 2);
    CHECK(recycledCount == 1);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - Generating then freeing a bunch of IDs", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    uint32_t recyclableCount;
    std::vector<foeId> idList;
    idList.reserve(256);

    for (int i = 0; i < 256; ++i) {
        foeId id;
        REQUIRE(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        idList.push_back(id);
    }

    REQUIRE(foeEcsExportIndexes(testIndexes, nullptr, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(recyclableCount == 0);
    REQUIRE(foeEcsFreeIDs(testIndexes, idList.size(), idList.data()).value == FOE_ECS_SUCCESS);

    REQUIRE(foeEcsExportIndexes(testIndexes, nullptr, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);
    REQUIRE(recyclableCount == 256);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - Reset with custom values", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    uint32_t recyclableCount;
    foeIdIndex nextIndex;

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 1);
    REQUIRE(recyclableCount == 0);

    foeId id;
    REQUIRE(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, id).value == FOE_ECS_SUCCESS);

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 2);
    REQUIRE(recyclableCount == 1);

    std::vector<foeIdIndex> list = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    };

    REQUIRE(foeEcsImportIndexes(testIndexes, 100, list.size(), list.data()).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 100);
    REQUIRE(recyclableCount == 10);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);

        REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
                FOE_ECS_SUCCESS);

        REQUIRE(nextIndex == 100);
        REQUIRE(recyclableCount == 9U - i);
        REQUIRE(id == foeId(i));
    }

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 100);
    REQUIRE(recyclableCount == 0);

    REQUIRE(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 101);
    REQUIRE(recyclableCount == 0);
    REQUIRE(id == foeId(100));

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - Import fails with a value less than the minimum index",
          "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    CHECK(foeEcsImportIndexes(testIndexes, foeIdIndexMinValue - 1, 0, nullptr).value ==
          FOE_ECS_ERROR_INDEX_BELOW_MINIMUM);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - GroupID of 0x0", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    // Generate the first ID
    foeId test0 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test0).value == FOE_ECS_SUCCESS);

    REQUIRE(test0 == foeId(0x00000001));
    REQUIRE(foeIdGetGroup(test0) == 0x00000000);
    REQUIRE(foeIdGroupToValue(test0) == 0x0);
    REQUIRE(foeIdGetIndex(test0) == 1);

    // Generate the second ID
    foeId test1 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test1).value == FOE_ECS_SUCCESS);

    REQUIRE(test1 == foeId(0x00000002));
    REQUIRE(foeIdGetGroup(test1) == 0x00000000);
    REQUIRE(foeIdGroupToValue(test1) == 0x0);
    REQUIRE(foeIdGetIndex(test1) == 2);

    // Recycle the second ID then first ID and generate again as their opposites
    REQUIRE(foeEcsFreeID(testIndexes, test1).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, test0).value == FOE_ECS_SUCCESS);

    test0 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test0).value == FOE_ECS_SUCCESS);
    THEN("First recycled should be 2 again.") {
        REQUIRE(test0 == foeId(0x00000002));
        REQUIRE(foeIdGetGroup(test0) == 0x00000000);
        REQUIRE(foeIdGroupToValue(test0) == 0x0);
        REQUIRE(foeIdGetIndex(test0) == 2);
    }

    test1 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test1).value == FOE_ECS_SUCCESS);
    THEN("Second recycled should be 1 again.") {
        REQUIRE(test1 == foeId(0x00000001));
        REQUIRE(foeIdGetGroup(test1) == 0x00000000);
        REQUIRE(foeIdGroupToValue(test1) == 0x0);
        REQUIRE(foeIdGetIndex(test1) == 1);
    }

    WHEN("Should fail to free an ID not from correct groupID.") {
        REQUIRE(foeEcsFreeID(testIndexes, foeIdValueToGroup(0xE)).value ==
                FOE_ECS_ERROR_INCORRECT_GROUP_ID);
    }
    WHEN("Should fail to free an InvalidID") {
        REQUIRE(foeEcsFreeID(testIndexes, FOE_INVALID_ID).value == FOE_ECS_ERROR_INVALID_ID);
    }

    SECTION("Should return an Invalid ID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        REQUIRE(foeEcsImportIndexes(testIndexes, foeIdIndexMaxValue - 10001, 0, nullptr).value ==
                FOE_ECS_SUCCESS);

        for (uint64_t i = 0; i < 10001; ++i) {
            foeId temp = FOE_INVALID_ID;
            REQUIRE(foeEcsGenerateID(testIndexes, &temp).value == FOE_ECS_SUCCESS);
            if (temp == FOE_INVALID_ID) {
                REQUIRE(temp != FOE_INVALID_ID);
            }
        }

        foeId temp = FOE_INVALID_ID;
        REQUIRE(foeEcsGenerateID(testIndexes, &temp).value == FOE_ECS_ERROR_OUT_OF_INDEXES);
        REQUIRE(temp == FOE_INVALID_ID);

        uint32_t recyclableCount;
        foeIdIndex nextIndex;

        REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
                FOE_ECS_SUCCESS);

        REQUIRE(nextIndex == foeIdIndexMaxValue);
    }

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - GroupID of 0xF", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0xF0000000, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    REQUIRE(foeEcsIndexesGetGroupID(testIndexes) == 0xF0000000);

    // Generate the first ID
    foeId test0 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test0).value == FOE_ECS_SUCCESS);

    REQUIRE(test0 == foeId(0xF0000001));
    REQUIRE(foeIdGetGroup(test0) == 0xF0000000);
    REQUIRE(foeIdGroupToValue(test0) == 0xF);
    REQUIRE(foeIdGetIndex(test0) == 1);

    // Generate the second ID
    foeId test1 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test1).value == FOE_ECS_SUCCESS);

    REQUIRE(test1 == foeId(0xF0000002));
    REQUIRE(foeIdGetGroup(test1) == 0xF0000000);
    REQUIRE(foeIdGroupToValue(test1) == 0xF);
    REQUIRE(foeIdGetIndex(test1) == 2);

    // Recycle the second ID then first ID and generate again as their opposites
    REQUIRE(foeEcsFreeID(testIndexes, test1).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, test0).value == FOE_ECS_SUCCESS);

    test0 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test0).value == FOE_ECS_SUCCESS);

    THEN("First recycled should be 2 again.") {
        REQUIRE(test0 == foeId(0xF0000002));
        REQUIRE(foeIdGetGroup(test0) == 0xF0000000);
        REQUIRE(foeIdGroupToValue(test0) == 0xF);
        REQUIRE(foeIdGetIndex(test0) == 2);
    }

    test1 = FOE_INVALID_ID;
    REQUIRE(foeEcsGenerateID(testIndexes, &test1).value == FOE_ECS_SUCCESS);

    THEN("Second recycled should be 1 again.") {
        REQUIRE(test1 == foeId(0xF0000001));
        REQUIRE(foeIdGetGroup(test1) == 0xF0000000);
        REQUIRE(foeIdGroupToValue(test1) == 0xF);
        REQUIRE(foeIdGetIndex(test1) == 1);
    }

    WHEN("Should fail to free an ID not from correct groupID.") {
        REQUIRE(foeEcsFreeID(testIndexes, foeId(0xF)).value == FOE_ECS_ERROR_INCORRECT_GROUP_ID);
    }
    WHEN("Should fail to free an InvalidID") {
        REQUIRE(foeEcsFreeID(testIndexes, FOE_INVALID_ID).value == FOE_ECS_ERROR_INVALID_ID);
    }

    SECTION("Should return an Invalid ID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        REQUIRE(foeEcsImportIndexes(testIndexes, foeIdIndexMaxValue - 10001, 0, nullptr).value ==
                FOE_ECS_SUCCESS);

        for (uint64_t i = 0; i < 10001; ++i) {
            foeId temp = FOE_INVALID_ID;
            REQUIRE(foeEcsGenerateID(testIndexes, &temp).value == FOE_ECS_SUCCESS);
            if (temp == FOE_INVALID_ID) {
                REQUIRE(temp != FOE_INVALID_ID);
            }
        }

        foeId temp = FOE_INVALID_ID;
        REQUIRE(foeEcsGenerateID(testIndexes, &temp).value == FOE_ECS_ERROR_OUT_OF_INDEXES);
        REQUIRE(temp == FOE_INVALID_ID);

        uint32_t recyclableCount;
        foeIdIndex nextIndex;

        REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
                FOE_ECS_SUCCESS);

        REQUIRE(nextIndex == foeIdIndexMaxValue);
    }

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - Attempting to free incorrect/invalid IDs from list",
          "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0xF0000000, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    SECTION("Invalid ID") {
        REQUIRE(foeEcsFreeID(testIndexes, FOE_INVALID_ID).value == FOE_ECS_ERROR_INVALID_ID);
    }

    SECTION("Different GroupID") {
        REQUIRE(foeEcsFreeID(testIndexes, 0xE0000000).value == FOE_ECS_ERROR_INCORRECT_GROUP_ID);
    }

    SECTION("Higher ID than given out") {
        REQUIRE(foeEcsFreeID(testIndexes, 0xF0000015).value == FOE_ECS_ERROR_INDEX_ABOVE_GENERATED);
    }

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - Iterating through IDs", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0x0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    for (int i = 0; i < 15; ++i) {
        foeId temp = FOE_INVALID_ID;
        REQUIRE(foeEcsGenerateID(testIndexes, &temp).value == FOE_ECS_SUCCESS);
    }

    uint32_t recyclableCount;
    foeIdIndex nextIndex;

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recyclableCount, nullptr).value ==
            FOE_ECS_SUCCESS);

    REQUIRE(nextIndex == 16);

    REQUIRE(foeEcsFreeID(testIndexes, 15).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, 8).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, 4).value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsFreeID(testIndexes, 10).value == FOE_ECS_SUCCESS);

    std::vector<foeId> existingIDs;

    foeEcsForEachID(
        testIndexes,
        [](void *pContext, foeId id) {
            static_cast<std::vector<foeId> *>(pContext)->emplace_back(id);
        },
        (void *)&existingIDs);

    REQUIRE(existingIDs.size() == 11);
    CHECK(existingIDs[0] == 1);
    CHECK(existingIDs[1] == 2);
    CHECK(existingIDs[2] == 3);
    CHECK(existingIDs[3] == 5);
    CHECK(existingIDs[4] == 6);
    CHECK(existingIDs[5] == 7);
    CHECK(existingIDs[6] == 9);
    CHECK(existingIDs[7] == 11);
    CHECK(existingIDs[8] == 12);
    CHECK(existingIDs[9] == 13);
    CHECK(existingIDs[10] == 14);

    foeEcsDestroyIndexes(testIndexes);
}

TEST_CASE("foeEcsIndexes - ImexData import/export", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes{FOE_NULL_HANDLE};

    REQUIRE(foeEcsCreateIndexes(0, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    for (int i = 0; i < 15; ++i) {
        foeId id;
        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
    }

    foeIdIndex nextIndex;
    uint32_t recycledCount;

    REQUIRE(foeEcsExportIndexes(testIndexes, &nextIndex, &recycledCount, nullptr).value ==
            FOE_ECS_SUCCESS);
    REQUIRE(nextIndex == 16);

    CHECK(foeEcsFreeID(testIndexes, 8).value == FOE_ECS_SUCCESS);
    CHECK(foeEcsFreeID(testIndexes, 4).value == FOE_ECS_SUCCESS);
    CHECK(foeEcsFreeID(testIndexes, 10).value == FOE_ECS_SUCCESS);

    SECTION("Current state can be overwritten") {
        foeIdIndex nextNewIndex = 10905;
        std::vector<foeIdIndex> recycledIds = {109, 4, 1000, 9876};

        CHECK(foeEcsImportIndexes(testIndexes, nextNewIndex, recycledIds.size(), recycledIds.data())
                  .value == FOE_ECS_SUCCESS);

        foeId id;

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 109);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 4);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 1000);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 9876);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 10905);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 10906);
    }
    SECTION("With ImexData export/import") {
        foeIdIndex nextNewIndex;
        std::vector<foeIdIndex> recycledIds;

        uint32_t count;
        REQUIRE(foeEcsExportIndexes(testIndexes, nullptr, &count, nullptr).value ==
                FOE_ECS_SUCCESS);
        CHECK(count == 3);

        recycledIds.resize(count);

        SECTION("If the output array is too small, then FOE_ECS_INCOMPLETE is returned") {
            uint32_t tempCount = 2;
            REQUIRE(foeEcsExportIndexes(testIndexes, &nextNewIndex, &tempCount, recycledIds.data())
                        .value == FOE_ECS_INCOMPLETE);
        }

        REQUIRE(foeEcsExportIndexes(testIndexes, &nextNewIndex, &count, recycledIds.data()).value ==
                FOE_ECS_SUCCESS);

        foeEcsIndexes testIndexes2{FOE_NULL_HANDLE};
        REQUIRE(foeEcsCreateIndexes(0, &testIndexes2).value == FOE_ECS_SUCCESS);
        CHECK(testIndexes2 != FOE_NULL_HANDLE);

        REQUIRE(
            foeEcsImportIndexes(testIndexes2, nextNewIndex, recycledIds.size(), recycledIds.data())
                .value == FOE_ECS_SUCCESS);

        foeId id;

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 8);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 4);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 10);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 16);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 17);

        foeEcsDestroyIndexes(testIndexes2);
    }
    SECTION("Without export/import") {
        foeId id;

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 8);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 4);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 10);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 16);

        CHECK(foeEcsGenerateID(testIndexes, &id).value == FOE_ECS_SUCCESS);
        CHECK(id == 17);
    }

    foeEcsDestroyIndexes(testIndexes);
}

namespace {

constexpr auto cNumIds = 256;
constexpr auto cNumThreads = 2;

static_assert(cNumThreads > 1, "Number of threads for tests must be > 1.");

void generateIds(std::vector<foeId> *idList, foeEcsIndexes idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        foeId id;
        foeEcsGenerateID(idGenerator, &id);
        idList->emplace_back(id);
    }
}

void freeIds(std::vector<foeId> *idList, foeEcsIndexes idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        foeEcsFreeID(idGenerator, (*idList)[i]);
    }
}

} // namespace

TEST_CASE("foeEcsIndexes - Multi-threaded synchronization tests", "[foe][ecs][foeEcsIndexes]") {
    foeEcsIndexes testIndexes = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateIndexes(0xF0000000, &testIndexes).value == FOE_ECS_SUCCESS);
    CHECK(testIndexes != FOE_NULL_HANDLE);

    std::vector<foeId> idList[cNumThreads];
    for (auto &i : idList) {
        i.reserve(cNumIds);
    }

    std::thread threads[cNumThreads];

    for (int i = 0; i < cNumThreads; ++i) {
        threads[i] = std::thread(generateIds, &idList[i], testIndexes);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::vector<foeId> fullList;
    fullList.reserve(cNumThreads * cNumIds);

    for (auto &i : idList) {
        fullList.insert(fullList.end(), i.begin(), i.end());
    }
    std::sort(fullList.begin(), fullList.end());

    REQUIRE(fullList.size() == cNumThreads * cNumIds);
    REQUIRE(std::unique(fullList.begin(), fullList.end()) == fullList.end());

    SECTION("Recycling all IDs in multi-thread doesn't lead to issues") {
        for (int i = 0; i < cNumThreads; ++i) {
            threads[i] = std::thread(freeIds, &idList[i], testIndexes);
        }

        for (auto &thread : threads) {
            thread.join();
        }

        uint32_t count;
        REQUIRE(foeEcsExportIndexes(testIndexes, nullptr, &count, nullptr).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(count == cNumThreads * cNumIds);
    }

    SECTION("Recycling all IDs in multi-thread and generating new ones doesn't lead to issues") {
        std::thread newThreads[cNumThreads];
        std::vector<foeId> newList[cNumThreads];
        for (auto &id : newList) {
            id.reserve(cNumIds);
        }

        for (int i = 0; i < cNumThreads; ++i) {
            threads[i] = std::thread(freeIds, &idList[i], testIndexes);
            newThreads[i] = std::thread(generateIds, &newList[i], testIndexes);
        }

        for (int i = 0; i < cNumThreads; ++i) {
            threads[i].join();
            newThreads[i].join();
        }

        std::vector<foeId> fullList;
        fullList.reserve(cNumThreads * cNumIds);

        for (auto &id : idList) {
            fullList.insert(fullList.end(), id.begin(), id.end());
        }
        std::sort(fullList.begin(), fullList.end());

        REQUIRE(fullList.size() == cNumThreads * cNumIds);
        REQUIRE(std::unique(fullList.begin(), fullList.end()) == fullList.end());
    }

    foeEcsDestroyIndexes(testIndexes);
}