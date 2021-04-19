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

#include "group_data.hpp"

#include <string>

class DummyImporter : public foeImporterBase {
  public:
    DummyImporter(std::string_view name, foeIdGroup group, void *resourceReturn = nullptr) :
        mName{name}, mGroup{group}, mResReturn{resourceReturn} {}

    foeIdGroup group() const noexcept final { return mGroup; }
    std::string name() const noexcept final { return mName; }
    void setGroupTranslator(foeIdGroupTranslator &&groupTranslation) final {}

    bool getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) final { return false; }
    bool getGroupIndexData(foeIdIndexGenerator &ecsGroup) final { return false; }
    bool importStateData(StatePools *pStatePools) final { return false; }

    bool importResourceDefinitions(ResourcePools *pResourcePools,
                                   ResourceLoaders *pResourceLoaders) {
        return false;
    }
    foeResourceCreateInfoBase *getResource(foeId id) final {
        return static_cast<foeResourceCreateInfoBase *>(mResReturn);
    }

  private:
    std::string mName;
    foeIdGroup mGroup;
    void *mResReturn;
};

TEST_CASE("foeGroupData - Initial State", "[foe]") {
    foeGroupData test;

    SECTION("Persistent Group") {
        REQUIRE(test.persistentIndices() != nullptr);
        REQUIRE(test.persistentIndices() == test.indices(foeIdPersistentGroup));
        REQUIRE(test.persistentIndices()->groupID() == foeIdPersistentGroup);
        REQUIRE(test.persistentIndices() == test.indices("Persistent"));

        REQUIRE(test.indices(foeIdPersistentGroup) != nullptr);
        REQUIRE(test.indices(foeIdPersistentGroup) == test.indices(foeIdPersistentGroup));
        REQUIRE(test.indices(foeIdPersistentGroup)->groupID() == foeIdPersistentGroup);
        REQUIRE(test.indices(foeIdPersistentGroup) == test.indices("Persistent"));

        REQUIRE(test.persistentImporter() == nullptr);
        REQUIRE(test.persistentImporter() == test.importer(foeIdPersistentGroup));
        REQUIRE(test.persistentImporter() == test.importer("Persistent"));

        REQUIRE(test.importer(foeIdPersistentGroup) == nullptr);
    }

    SECTION("Temporary Group") {
        REQUIRE(test.temporaryIndices() != nullptr);
        REQUIRE(test.temporaryIndices() == test.indices(foeIdTemporaryGroup));
        REQUIRE(test.temporaryIndices()->groupID() == foeIdTemporaryGroup);
        REQUIRE(test.temporaryIndices() == test.indices("Temporary"));

        REQUIRE(test.indices(foeIdTemporaryGroup) != nullptr);
        REQUIRE(test.indices(foeIdTemporaryGroup) == test.indices(foeIdTemporaryGroup));
        REQUIRE(test.indices(foeIdTemporaryGroup)->groupID() == foeIdTemporaryGroup);
        REQUIRE(test.indices(foeIdTemporaryGroup) == test.indices("Temporary"));

        REQUIRE(test.importer(foeIdTemporaryGroup) == nullptr);
    }

    SECTION("Dynamic Groups") {
        for (uint32_t i = 0; i < (foeIdGroupMaxValue - 2); ++i) {
            REQUIRE(test.indices(foeIdValueToGroup(i)) == nullptr);
            REQUIRE(test.importer(foeIdValueToGroup(i)) == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - addDynamicGroup failure cases", "[foe]") {
    foeGroupData test;

    SECTION("No IndexGenerator given") {
        std::unique_ptr<foeIdIndexGenerator> testIndices;
        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("No Importer given") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        std::unique_ptr<DummyImporter> testImporter;

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Blank name in the importer") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Inconsistent ID Groups") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Persistent' ID Group") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdPersistentGroup);
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdPersistentGroup);

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Persistent' reserved name") {
        auto testIndices =
            std::make_unique<foeIdIndexGenerator>("Persistent", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("Persistent", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Temporary' ID Group") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdTemporaryGroup);
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdTemporaryGroup);

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Same as 'Temporary' reserved name") {
        auto testIndices =
            std::make_unique<foeIdIndexGenerator>("Temporary", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("Temporary", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Adding the same ID group more than once fails") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));

        testIndices = std::make_unique<foeIdIndexGenerator>("b", foeIdValueToGroup(0x1));
        testImporter = std::make_unique<DummyImporter>("b", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }

    SECTION("Adding the same Importer group name more than once fails") {
        auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));

        testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x2));
        testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
    }
}

TEST_CASE("foeGroupData - setPersistentImporter failure cases", "[foe]") {
    foeGroupData test;

    SECTION("Giving a nullptr") { REQUIRE_FALSE(test.setPersistentImporter(nullptr)); }

    SECTION("Giving an importer that is not foeIdPersistentGroup") {
        SECTION("Dynamic Groups") {
            for (foeIdGroup group = 0; group < foeIdMaxDynamicGroups; ++group) {
                auto tempImporter = std::make_unique<DummyImporter>("testPersistent", group);

                REQUIRE_FALSE(test.setPersistentImporter(std::move(tempImporter)));
            }
        }

        SECTION("foeIdTemporaryGroup") {
            auto tempImporter =
                std::make_unique<DummyImporter>("testPersistent", foeIdTemporaryGroup);
            REQUIRE_FALSE(test.setPersistentImporter(std::move(tempImporter)));
        }
    }
}

TEST_CASE("foeGroupData - Persistent/Temporary group IndexGenerator/Importer retrieval", "[foe]") {
    foeGroupData test;

    auto persistentImporter =
        std::make_unique<DummyImporter>("testPersistent", foeIdPersistentGroup);
    test.setPersistentImporter(std::move(persistentImporter));

    SECTION("Persistent Data") {
        REQUIRE(test.persistentIndices() != nullptr);
        REQUIRE(test.persistentIndices()->groupID() == foeIdPersistentGroup);

        REQUIRE(test.persistentImporter() != nullptr);
        REQUIRE(test.persistentImporter()->group() == foeIdPersistentGroup);
        REQUIRE(test.persistentImporter()->name() == "testPersistent");

        REQUIRE(test.importer(foeIdPersistentGroup) != nullptr);
        REQUIRE(test.importer("testPersistent") != nullptr);
    }

    SECTION("Temporary Data") {
        REQUIRE(test.temporaryIndices() != nullptr);
        REQUIRE(test.temporaryIndices()->groupID() == foeIdTemporaryGroup);
    }
}

TEST_CASE("foeGroupData - IndexGenerator/Importer retrieval", "[foe]") {
    foeGroupData test;

    auto testIndices = std::make_unique<foeIdIndexGenerator>("0x1", foeIdValueToGroup(0x1));
    auto testImporter = std::make_unique<DummyImporter>("0x1", foeIdValueToGroup(0x1));
    REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));

    SECTION("0x1 Group Data") {
        REQUIRE(test.indices(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.indices("0x1") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.importer("0x1") != nullptr);
    }

    testIndices = std::make_unique<foeIdIndexGenerator>(
        "MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    testImporter =
        std::make_unique<DummyImporter>("MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));

    SECTION("MaxDynamic Group Data") {
        REQUIRE(test.indices(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.indices("MaxDynamic") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.importer("MaxDynamic") != nullptr);
    }

    SECTION("Failure cases") {
        SECTION("Temporary importer by name fails") {
            REQUIRE(test.importer("Temporary") == nullptr);
        }

        SECTION("Name not added") {
            REQUIRE(test.indices("foeIdMaxDynamicGroups") == nullptr);
            REQUIRE(test.indices("foeIdMaxDynamicGroups") == nullptr);

            REQUIRE(test.importer("foeIdMaxDynamicGroups") == nullptr);
            REQUIRE(test.importer("foeIdMaxDynamicGroups") == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - getResourceDefinition", "[foe]") {
    foeGroupData test;
    foeResourceCreateInfoBase *pCreateInfo;

    SECTION("No importers always fails") {
        REQUIRE_FALSE(test.getResourceDefinition(FOE_INVALID_ID, &pCreateInfo));
    }

    SECTION("Persistent importer added") {
        SECTION("Returning true") {
            auto persistentImporter = std::make_unique<DummyImporter>(
                "testPersistent", foeIdPersistentGroup, static_cast<int *>(NULL) + 1);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID, &pCreateInfo));
        }
        SECTION("Returning false") {
            auto persistentImporter =
                std::make_unique<DummyImporter>("testPersistent", foeIdPersistentGroup);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE_FALSE(test.getResourceDefinition(FOE_INVALID_ID, &pCreateInfo));
        }
    }

    SECTION("Dynamic importer added") {
        SECTION("Returning true") {
            auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1),
                                                                static_cast<int *>(NULL) + 1);

            REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID, &pCreateInfo));
        }
        SECTION("Returning false") {
            auto testIndices = std::make_unique<foeIdIndexGenerator>("a", foeIdValueToGroup(0x1));
            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

            REQUIRE(test.addDynamicGroup(std::move(testIndices), std::move(testImporter)));
            REQUIRE_FALSE(test.getResourceDefinition(FOE_INVALID_ID, &pCreateInfo));
        }
    }
}