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
#include <foe/ecs/groups.hpp>
#include <foe/ecs/id.hpp>
#include <foe/ecs/index_generator.hpp>

TEST_CASE("Groups - Getting groups", "[foe][ecs]") {
    foeEcsGroups test;

    SECTION("Persistent") {
        REQUIRE(test.persistentGroup() != nullptr);
        REQUIRE(test.persistentGroup() == test.group(foePersistentGroup));
        REQUIRE(test.persistentGroup()->groupID() == foePersistentGroup);
        REQUIRE(test.persistentGroup() == test.group("Persistent"));
    }
    SECTION("Temporary") {
        REQUIRE(test.temporaryGroup() != nullptr);
        REQUIRE(test.temporaryGroup() == test.group(foeTemporaryGroup));
        REQUIRE(test.temporaryGroup()->groupID() == foeTemporaryGroup);
        REQUIRE(test.temporaryGroup() == test.group("Temporary"));
    }
    SECTION("General") {
        for (uint32_t i = 0; i < (foeEcsMaxGroupValue - 2); ++i) {
            REQUIRE(test.group(i << foeEcsNumIndexBits) == nullptr);
        }
    }
    SECTION("Invalid GroupID") { REQUIRE(test.group(0x1) == nullptr); }
}

TEST_CASE("Groups - Add/get/remove operations") {
    foeEcsGroups test;

    auto gen1 = std::make_unique<foeEcsIndexGenerator>("A", 0xA0000000);
    auto *pGen1 = gen1.get();
    auto gen2 = std::make_unique<foeEcsIndexGenerator>("B", 0xB0000000);
    auto *pGen2 = gen2.get();

    REQUIRE(test.addGroup(std::move(gen1)));
    REQUIRE(test.addGroup(std::move(gen2)));

    REQUIRE(test.group(0xA0000000) == pGen1);
    REQUIRE(test.group("A") == pGen1);

    REQUIRE(test.group(0xB0000000) == pGen2);
    REQUIRE(test.group("B") == pGen2);

    REQUIRE(test.group(0xC0000000) == nullptr);
    REQUIRE(test.group("C") == nullptr);

    gen1 = std::make_unique<foeEcsIndexGenerator>("A", 0xA0000000);
    SECTION("Re-adding the same group fails") { REQUIRE_FALSE(test.addGroup(std::move(gen1))); }
    SECTION("Adding a different group with the same GroupID fails") {
        auto dupGroup = std::make_unique<foeEcsIndexGenerator>("C", 0xA0000000);
        REQUIRE_FALSE(test.addGroup(std::move(dupGroup)));
    }
    SECTION("Adding a different group with the same name fails") {
        auto dupGroup = std::make_unique<foeEcsIndexGenerator>("A", 0xC0000000);
        REQUIRE_FALSE(test.addGroup(std::move(dupGroup)));
    }

    test.removeGroup(0xA0000000);
    REQUIRE(test.group(0xA0000000) == nullptr);
    REQUIRE(test.group(0xB0000000) == pGen2);

    test.removeGroup(0xB0000000);
    REQUIRE(test.group(0xA0000000) == nullptr);
    REQUIRE(test.group(0xB0000000) == nullptr);
}

TEST_CASE("Groups - Adding temporary/persistent GroupIDs fail", "[foe][ecs]") {
    foeEcsGroups test;

    SECTION("Same GroupID") {
        auto pGroup = std::make_unique<foeEcsIndexGenerator>("", foePersistentGroup);
        REQUIRE_FALSE(test.addGroup(std::move(pGroup)));

        auto tGroup = std::make_unique<foeEcsIndexGenerator>("", foeTemporaryGroup);
        REQUIRE_FALSE(test.addGroup(std::move(tGroup)));
    }
    SECTION("Same name") {
        auto pGroup = std::make_unique<foeEcsIndexGenerator>("Persistent", 0);
        REQUIRE_FALSE(test.addGroup(std::move(pGroup)));

        auto tGroup = std::make_unique<foeEcsIndexGenerator>("Temporary", 0);
        REQUIRE_FALSE(test.addGroup(std::move(tGroup)));
    }
}

TEST_CASE("Groups - Removing temporary/persistent GroupIDs fail", "[foe][ecs]") {
    foeEcsGroups test;

    test.removeGroup(foePersistentGroup);
    test.removeGroup(foeTemporaryGroup);

    REQUIRE(test.group(foePersistentGroup) != nullptr);
    REQUIRE(test.group(foeTemporaryGroup) != nullptr);
}

TEST_CASE("Groups - Adding/removing all possible general groups", "[foe][ecs]") {
    foeEcsGroups test;

    std::array<std::unique_ptr<foeEcsIndexGenerator>, foeMaxGeneralGroups> groups;

    for (uint32_t i = 0; i < foeMaxGeneralGroups; ++i) {
        groups[i] =
            std::make_unique<foeEcsIndexGenerator>(std::to_string(i), foeEcsNormalizedToGroupID(i));
    }

    for (auto &group : groups) {
        auto id = group->groupID();
        auto *pNewGroup = group.get();

        REQUIRE(test.addGroup(std::move(group)));
        REQUIRE(test.group(id) == pNewGroup);
    }

    for (uint32_t i = 0; i < foeMaxGeneralGroups; ++i) {
        const foeIdGroup id = foeEcsNormalizedToGroupID(i);

        REQUIRE(test.group(id) != nullptr);
        REQUIRE(test.group(id)->groupID() == id);
        REQUIRE(test.group(id)->name() == std::to_string(i));
        test.removeGroup(id);
        REQUIRE(test.group(id) == nullptr);
    }
}
