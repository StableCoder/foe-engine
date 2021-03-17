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
    foeEcsIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);
    REQUIRE(test.name() == "");
}

TEST_CASE("IndexGenerator - name", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("test", 0);

    REQUIRE(test.name() == "test");
}

TEST_CASE("IndexGenerator - plain reset", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);
}

TEST_CASE("IndexGenerator - reset with initial values", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);
}

TEST_CASE("IndexGenerator - Generating then freeing a bunch of IDs", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    std::vector<foeEntityID> idList;
    idList.reserve(256);

    for (int i = 0; i < 256; ++i) {
        idList.push_back(test.generate());
    }

    REQUIRE(test.recyclable() == 0);
    test.free(idList.size(), idList.data());
    REQUIRE(test.recyclable() == 256);
}

TEST_CASE("IndexGenerator - Reset with custom values", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    test.free(test.generate());

    REQUIRE(test.peekNextFreshIndex() == 1);
    REQUIRE(test.recyclable() == 1);

    std::vector<foeIndexID> list = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    };

    test.importState(100, list);
    REQUIRE(test.peekNextFreshIndex() == 100);
    REQUIRE(test.recyclable() == 10);

    for (int i = 0; i < 10; ++i) {
        auto id = test.generate();
        REQUIRE(test.peekNextFreshIndex() == 100);
        REQUIRE(test.recyclable() == 9 - i);
        REQUIRE(id == foeEntityID(i));
    }

    REQUIRE(test.peekNextFreshIndex() == 100);
    REQUIRE(test.recyclable() == 0);

    auto id = test.generate();

    REQUIRE(test.peekNextFreshIndex() == 101);
    REQUIRE(test.recyclable() == 0);
    REQUIRE(id == foeEntityID(100));
}

TEST_CASE("IndexGenerator - GroupID of 0x0", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    REQUIRE(test.groupID() == 0);
    REQUIRE(test.peekNextFreshIndex() == 0);
    REQUIRE(test.recyclable() == 0);

    // Generate first ID
    foeEntityID test0 = test.generate();

    REQUIRE(test0 == foeEntityID(0));
    REQUIRE(foeEcsGetGroupID(test0) == 0);
    REQUIRE(foeEcsGetNormalizedGroupID(test0) == 0);
    REQUIRE(foeEcsGetIndexID(test0) == 0);
    REQUIRE(test.peekNextFreshIndex() == 1);

    // Generate second ID
    foeEntityID test1 = test.generate();

    REQUIRE(test1 == foeEntityID(1));
    REQUIRE(foeEcsGetGroupID(test1) == 0);
    REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0);
    REQUIRE(foeEcsGetIndexID(test1) == 1);
    REQUIRE(test.peekNextFreshIndex() == 2);

    // Recycle the second ID and generate again.
    REQUIRE(test.free(test1));

    test1 = test.generate();

    THEN("Should be 1 again.") {
        REQUIRE(test1 == foeEntityID(1));
        REQUIRE(foeEcsGetGroupID(test1) == 0);
        REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0);
        REQUIRE(foeEcsGetIndexID(test1) == 1);
        REQUIRE(test.peekNextFreshIndex() == 2);
    }

    WHEN("Should fail to free an ID not from correct groupID.") {
        REQUIRE(!test.free(foeEntityID(0xF)));
    }
    WHEN("Should fail to free an InvalidID") { REQUIRE(!test.free(FOE_INVALID_ENTITY)); }

    SECTION("Should return an Invalid ID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        test.importState(foeEcsMaxIndexValue - 10000, {});

        for (uint64_t i = 0; i < 10001; ++i) {
            if (auto temp = test.generate(); temp == FOE_INVALID_ENTITY) {
                REQUIRE(temp != FOE_INVALID_ENTITY);
            }
        }

        REQUIRE(test.generate() == FOE_INVALID_ENTITY);
        REQUIRE(test.peekNextFreshIndex() == foeEcsInvalidIndexID);
    }
}

TEST_CASE("IndexGenerator - GroupID of 0xF0", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0xF0000000);

    REQUIRE(test.groupID() == 0xF0000000);

    // Generate the first ID
    foeEntityID test0 = test.generate();

    REQUIRE(test0 == foeEntityID(0xF0000000));
    REQUIRE(foeEcsGetGroupID(test0) == 0xF0000000);
    REQUIRE(foeEcsGetNormalizedGroupID(test0) == 0xF);
    REQUIRE(foeEcsGetIndexID(test0) == 0);

    // Generate the second ID
    foeEntityID test1 = test.generate();

    REQUIRE(test1 == foeEntityID(0xF0000001));
    REQUIRE(foeEcsGetGroupID(test1) == 0xF0000000);
    REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0xF);
    REQUIRE(foeEcsGetIndexID(test1) == 1);

    // Recycle the second ID and generate again.
    REQUIRE(test.free(test1));

    test1 = test.generate();

    THEN("Should be 1 again.") {
        REQUIRE(test1 == foeEntityID(0xF0000001));
        REQUIRE(foeEcsGetGroupID(test1) == 0xF0000000);
        REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0xF);
        REQUIRE(foeEcsGetIndexID(test1) == 1);
    }

    WHEN("Should fail to free an ID not from correct groupID.") {
        REQUIRE(!test.free(foeEntityID(0xE)));
    }
    WHEN("Should fail to free an eInvalidID") { REQUIRE(!test.free(foeEntityID())); }

    SECTION("Should return an eInvalidID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        test.importState(foeEcsMaxIndexValue - 10000, {});

        for (uint64_t i = 0; i < 10001; ++i) {
            if (auto temp = test.generate(); temp == FOE_INVALID_ENTITY) {
                REQUIRE(temp != FOE_INVALID_ENTITY);
            }
        }

        REQUIRE(test.generate() == FOE_INVALID_ENTITY);
        REQUIRE(test.peekNextFreshIndex() == foeEcsInvalidIndexID);
    }
}

TEST_CASE("IndexGenerator - GroupID of 0xF", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0xF0000000);

    REQUIRE(test.groupID() == 0xF0000000);

    // Generate the first ID
    foeEntityID test0 = test.generate();

    REQUIRE(test0 == foeEntityID(0xF0000000));
    REQUIRE(foeEcsGetGroupID(test0) == 0xF0000000);
    REQUIRE(foeEcsGetNormalizedGroupID(test0) == 0xF);
    REQUIRE(foeEcsGetIndexID(test0) == 0);

    // Generate the second ID
    foeEntityID test1 = test.generate();

    REQUIRE(test1 == foeEntityID(0xF0000001));
    REQUIRE(foeEcsGetGroupID(test1) == 0xF0000000);
    REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0xF);
    REQUIRE(foeEcsGetIndexID(test1) == 1);

    // Recycle the second ID and generate again.
    REQUIRE(test.free(test1));

    test1 = test.generate();

    THEN("Should be 1 again.") {
        REQUIRE(test1 == foeEntityID(0xF0000001));
        REQUIRE(foeEcsGetGroupID(test1) == 0xF0000000);
        REQUIRE(foeEcsGetNormalizedGroupID(test1) == 0xF);
        REQUIRE(foeEcsGetIndexID(test1) == 1);
    }

    WHEN("Should fail to free an ID not from correct groupID.") {
        REQUIRE(!test.free(foeEntityID(0xE)));
    }
    WHEN("Should fail to free an eInvalidID") { REQUIRE(!test.free(FOE_INVALID_ENTITY)); }

    SECTION("Should return an Invalid ID when it runs out of indexes.") {
        // Shortcut to near the end of the index range
        test.importState(foeEcsMaxIndexValue - 10000, {});

        for (uint64_t i = 0; i < 10001; ++i) {
            if (auto temp = test.generate(); temp == FOE_INVALID_ENTITY) {
                REQUIRE(temp != FOE_INVALID_ENTITY);
            }
        }

        REQUIRE(test.generate() == FOE_INVALID_ENTITY);
        REQUIRE(test.peekNextFreshIndex() == foeEcsInvalidIndexID);
    }
}

TEST_CASE("IndexGenerator - Attempting to free incorrect/invalid IDs from list",
          "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0);

    SECTION("Invalid ID") {
        std::array<foeEntityID, 1> ids = {FOE_INVALID_ENTITY};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }

    SECTION("Different GroupID") {
        std::array<foeEntityID, 1> ids = {foeEntityID(0xF0000000)};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }

    SECTION("Higher ID than given out") {
        std::array<foeEntityID, 1> ids = {foeEntityID(0x15)};
        REQUIRE(!test.free(ids.size(), ids.data()));
    }
}

TEST_CASE("IndexGenerator - ImexData import/export", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator testGenerator{"", 0};

    for (int i = 0; i < 15; ++i) {
        testGenerator.generate();
    }

    REQUIRE(testGenerator.peekNextFreshIndex() == 15);

    testGenerator.free(8);
    testGenerator.free(4);
    testGenerator.free(10);

    SECTION("Current state can be overwritten") {
        foeIndexID nextFreeId = 10905;
        std::vector<foeIndexID> recycledIds = {109, 4, 1000, 9876};

        testGenerator.importState(nextFreeId, recycledIds);

        CHECK(testGenerator.generate() == 109);
        CHECK(testGenerator.generate() == 4);
        CHECK(testGenerator.generate() == 1000);
        CHECK(testGenerator.generate() == 9876);
        CHECK(testGenerator.generate() == 10905);
        CHECK(testGenerator.generate() == 10906);
    }
    SECTION("With ImexData export/import") {
        foeIndexID nextFreeId;
        std::vector<foeIndexID> recycledIds;

        testGenerator.exportState(nextFreeId, recycledIds);

        foeEcsIndexGenerator testGenerator2{"", 0};
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

void generateIds(std::vector<foeEntityID> *idList, foeEcsIndexGenerator *idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        idList->emplace_back(idGenerator->generate());
    }
}

void freeIds(std::vector<foeEntityID> *idList, foeEcsIndexGenerator *idGenerator) {
    for (int i = 0; i < cNumIds; ++i) {
        idGenerator->free((*idList)[i]);
    }
}

} // namespace

TEST_CASE("IndexGenerator - Multi-threaded synchronization tests", "[foe][ecs][IndexGenerator]") {
    foeEcsIndexGenerator test("", 0xF0000000);

    std::vector<foeEntityID> idList[cNumThreads];
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

    std::vector<foeEntityID> fullList;
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
        std::vector<foeEntityID> newList[cNumThreads];
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

        std::vector<foeEntityID> fullList;
        fullList.reserve(cNumThreads * cNumIds);

        for (auto &id : idList) {
            fullList.insert(fullList.end(), id.begin(), id.end());
        }
        std::sort(fullList.begin(), fullList.end());

        REQUIRE(fullList.size() == cNumThreads * cNumIds);
        REQUIRE(std::unique(fullList.begin(), fullList.end()) == fullList.end());
    }
}