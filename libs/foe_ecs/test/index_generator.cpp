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
#include <foe/ecs/index_generator.hpp>

#include <array>
#include <thread>
#include <vector>

TEST_CASE("IndexGenerator - default state", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);
    REQUIRE(test.name() == "");
}

TEST_CASE("IndexGenerator - name", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("test", 0);

    REQUIRE(test.name() == "test");
}

TEST_CASE("IndexGenerator - plain reset", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);
}

TEST_CASE("IndexGenerator - reset with initial values", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);
}

TEST_CASE("IndexGenerator - Generating then freeing a bunch of IDs", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    std::vector<foeId> idList;
    idList.reserve(256);

    for (int i = 0; i < 256; ++i) {
        idList.push_back(test.generate());
    }

    REQUIRE(test.recyclable() == 0);
    test.free(idList.size(), idList.data());
    REQUIRE(test.recyclable() == 256);
}

TEST_CASE("IndexGenerator - Reset with custom values", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);

    std::vector<foeIdIndex> list = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    };

    test.importState(100, list);
    REQUIRE(test.peekNextFreshIndex() == 100);
    REQUIRE(test.recyclable() == 10);

    for (int i = 0; i < 10; ++i) {
        auto id = test.generate();
        REQUIRE(test.peekNextFreshIndex() == 100);
        REQUIRE(test.recyclable() == 9 - i);
        REQUIRE(id == foeId(i));
    }

    REQUIRE(test.peekNextFreshIndex() == 100);
    REQUIRE(test.recyclable() == 0);

    auto id = test.generate();

    REQUIRE(test.peekNextFreshIndex() == 101);
    REQUIRE(test.recyclable() == 0);
    REQUIRE(id == foeId(100));
}

TEST_CASE("IndexGenerator - GroupID of 0x0", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    // Generate the first ID (as an entity)
    foeId test0 = test.generate();

    REQUIRE(test0 == foeId(0x00000000));
    REQUIRE(foeIdGetGroup(test0) == 0x00000000);
    REQUIRE(foeIdGroupToValue(test0) == 0x0);
    REQUIRE(foeIdIsEntity(test0));
    REQUIRE(foeIdGetIndex(test0) == 0);

    // Generate the second ID (as a resource)
    foeId test1 = test.generateResource();

    REQUIRE(test1 == foeId(0x08000001));
    REQUIRE(foeIdGetGroup(test1) == 0x00000000);
    REQUIRE(foeIdGroupToValue(test1) == 0x0);
    REQUIRE(foeIdIsResource(test1));
    REQUIRE(foeIdGetIndex(test1) == 1);

    // Recycle the second ID then first ID and generate again as their opposites
    REQUIRE(test.free(test1));
    REQUIRE(test.free(test0));

    test0 = test.generate();
    THEN("First recycled should be 1 again.") {
        REQUIRE(test0 == foeId(0x00000001));
        REQUIRE(foeIdGetGroup(test0) == 0x00000000);
        REQUIRE(foeIdGroupToValue(test0) == 0x0);
        REQUIRE(foeIdIsEntity(test0));
        REQUIRE(foeIdGetIndex(test0) == 1);
    }

    test1 = test.generateResource();
    THEN("Second recycled should be 0 again.") {
        REQUIRE(test1 == foeId(0x08000000));
        REQUIRE(foeIdGetGroup(test1) == 0x00000000);
        REQUIRE(foeIdGroupToValue(test1) == 0x0);
        REQUIRE(foeIdIsResource(test1));
        REQUIRE(foeIdGetIndex(test1) == 0);
    }

    WHEN("Should fail to free an ID not from correct groupID.") { REQUIRE(!test.free(foeId(0xF))); }
    WHEN("Should fail to free an InvalidID") { REQUIRE(!test.free(FOE_INVALID_ID)); }

    SECTION("Should return an Invalid ID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        test.importState(foeIdIndexMaxValue - 10001, {});

        for (uint64_t i = 0; i < 10001; ++i) {
            if (auto temp = test.generate(); temp == FOE_INVALID_ID) {
                REQUIRE(temp != FOE_INVALID_ID);
            }
        }

        REQUIRE(test.generate() == FOE_INVALID_ID);
        REQUIRE(test.peekNextFreshIndex() == foeIdIndexMaxValue);
    }
}

TEST_CASE("IndexGenerator - GroupID of 0xF", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0xF0000000);

    REQUIRE(test.groupID() == 0xF0000000);

    // Generate the first ID (as an entity)
    foeId test0 = test.generate();

    REQUIRE(test0 == foeId(0xF0000000));
    REQUIRE(foeIdGetGroup(test0) == 0xF0000000);
    REQUIRE(foeIdGroupToValue(test0) == 0xF);
    REQUIRE(foeIdIsEntity(test0));
    REQUIRE(foeIdGetIndex(test0) == 0);

    // Generate the second ID (as a resource)
    foeId test1 = test.generateResource();

    REQUIRE(test1 == foeId(0xF8000001));
    REQUIRE(foeIdGetGroup(test1) == 0xF0000000);
    REQUIRE(foeIdGroupToValue(test1) == 0xF);
    REQUIRE(foeIdIsResource(test1));
    REQUIRE(foeIdGetIndex(test1) == 1);

    // Recycle the second ID then first ID and generate again as their opposites
    REQUIRE(test.free(test1));
    REQUIRE(test.free(test0));

    test0 = test.generate();
    THEN("First recycled should be 1 again.") {
        REQUIRE(test0 == foeId(0xF0000001));
        REQUIRE(foeIdGetGroup(test0) == 0xF0000000);
        REQUIRE(foeIdGroupToValue(test0) == 0xF);
        REQUIRE(foeIdIsEntity(test0));
        REQUIRE(foeIdGetIndex(test0) == 1);
    }

    test1 = test.generateResource();
    THEN("Second recycled should be 0 again.") {
        REQUIRE(test1 == foeId(0xF8000000));
        REQUIRE(foeIdGetGroup(test1) == 0xF0000000);
        REQUIRE(foeIdGroupToValue(test1) == 0xF);
        REQUIRE(foeIdIsResource(test1));
        REQUIRE(foeIdGetIndex(test1) == 0);
    }

    WHEN("Should fail to free an ID not from correct groupID.") { REQUIRE(!test.free(foeId(0xE))); }
    WHEN("Should fail to free an eInvalidID") { REQUIRE(!test.free(foeId())); }

    SECTION("Should return an eInvalidID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        test.importState(foeIdIndexMaxValue - 10001, {});

        for (uint64_t i = 0; i < 10001; ++i) {
            if (auto temp = test.generate(); temp == FOE_INVALID_ID) {
                REQUIRE(temp != FOE_INVALID_ID);
            }
        }

        REQUIRE(test.generate() == FOE_INVALID_ID);
        REQUIRE(test.peekNextFreshIndex() == foeIdIndexMaxValue);
    }
}

TEST_CASE("IndexGenerator - Attempting to free incorrect/invalid IDs from list",
          "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0);

    SECTION("Invalid ID") {
        std::array<foeId, 1> ids = {FOE_INVALID_ID};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }

    SECTION("Different GroupID") {
        std::array<foeId, 1> ids = {foeId(0xF0000000)};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }

    SECTION("Higher ID than given out") {
        std::array<foeId, 1> ids = {foeId(0x15)};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }
}

TEST_CASE("IndexGenerator - ImexData import/export", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator testGenerator{"", 0};

    for (int i = 0; i < 15; ++i) {
        testGenerator.generate();
    }

    REQUIRE(testGenerator.peekNextFreshIndex() == 15);

    testGenerator.free(8);
    testGenerator.free(4);
    testGenerator.free(10);

    SECTION("Current state can be overwritten") {
        foeIdIndex nextFreeId = 10905;
        std::vector<foeIdIndex> recycledIds = {109, 4, 1000, 9876};

        testGenerator.importState(nextFreeId, recycledIds);

        CHECK(testGenerator.generate() == 109);
        CHECK(testGenerator.generate() == 4);
        CHECK(testGenerator.generate() == 1000);
        CHECK(testGenerator.generate() == 9876);
        CHECK(testGenerator.generate() == 10905);
        CHECK(testGenerator.generate() == 10906);
    }
    SECTION("With ImexData export/import") {
        foeIdIndex nextFreeId;
        std::vector<foeIdIndex> recycledIds;

        testGenerator.exportState(nextFreeId, recycledIds);

        foeIdIndexGenerator testGenerator2{"", 0};
        testGenerator2.importState(nextFreeId, recycledIds);

        CHECK(testGenerator2.generate() == 8);
        CHECK(testGenerator2.generate() == 4);
        CHECK(testGenerator2.generate() == 10);
        CHECK(testGenerator2.generate() == 15);
        CHECK(testGenerator2.generate() == 16);
    }
    SECTION("Without export/import") {
        CHECK(testGenerator.generate() == 8);
        CHECK(testGenerator.generate() == 4);
        CHECK(testGenerator.generate() == 10);
        CHECK(testGenerator.generate() == 15);
        CHECK(testGenerator.generate() == 16);
    }
}

namespace {

constexpr auto cNumIds = 256;
constexpr auto cNumThreads = 2;

static_assert(cNumThreads > 1, "Number of threads for tests must be > 1.");

void generateIds(std::vector<foeId> *idList, foeIdIndexGenerator *idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        idList->emplace_back(idGenerator->generate());
    }
}

void freeIds(std::vector<foeId> *idList, foeIdIndexGenerator *idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        idGenerator->free((*idList)[i]);
    }
}

} // namespace

TEST_CASE("IndexGenerator - Multi-threaded synchronization tests", "[foe][ecs][IndexGenerator]") {
    foeIdIndexGenerator test("", 0xF0000000);

    std::vector<foeId> idList[cNumThreads];
    for (auto &i : idList) {
        i.reserve(cNumIds);
    }

    std::thread threads[cNumThreads];

    for (int i = 0; i < cNumThreads; ++i) {
        threads[i] = std::thread(generateIds, &idList[i], &test);
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
            threads[i] = std::thread(freeIds, &idList[i], &test);
        }

        for (auto &thread : threads) {
            thread.join();
        }

        REQUIRE(test.recyclable() == cNumThreads * cNumIds);
    }

    SECTION("Recycling all IDs in multi-thread and generating new ones doesn't lead to issues") {
        std::thread newThreads[cNumThreads];
        std::vector<foeId> newList[cNumThreads];
        for (auto &id : newList) {
            id.reserve(cNumIds);
        }

        for (int i = 0; i < cNumThreads; ++i) {
            threads[i] = std::thread(freeIds, &idList[i], &test);
            newThreads[i] = std::thread(generateIds, &newList[i], &test);
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
}