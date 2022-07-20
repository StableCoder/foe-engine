// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/ecs/error_code.h>
#include <foe/ecs/id.h>
#include <foe/imex/error_code.h>
#include <foe/simulation/group_data.hpp>

#include <filesystem>
#include <string>

static_assert(FOE_SIMULATION_TEST_DATA_DIR != NULL,
              "FOE_SIMULATION_TEST_DATA_DIR cannot be left undefined, check that it is defined in "
              "CMake script!");

class DummyImporter : public foeImporterBase {
  public:
    DummyImporter(std::string_view name, foeIdGroup group, void *resourceReturn = nullptr) :
        mName{name}, mGroup{group}, mResReturn{resourceReturn} {}

    foeIdGroup group() const noexcept final { return mGroup; }
    char const *name() const noexcept final { return mName.c_str(); }
    void setGroupTranslator(foeEcsGroupTranslator groupTranslator) final {}

    foeResult getDependencies(uint32_t *pDependencyCount,
                              foeIdGroup *pDependencyGroups,
                              uint32_t *pNamesLength,
                              char *pNames) {
        return foeResult{};
    }
    bool getGroupEntityIndexData(foeEcsIndexes indexes) final { return false; }
    bool getGroupResourceIndexData(foeEcsIndexes indexes) final { return false; }
    bool importStateData(foeEcsNameMap nameMap, foeSimulation const *pSimulation) final {
        return false;
    }

    bool importResourceDefinitions(foeEcsNameMap nameMap, foeSimulation const *pSimulation) {
        return false;
    }
    std::string getResourceEditorName(foeIdIndex resourceIndexID) { return std::string{}; }
    foeResourceCreateInfo getResource(foeId id) final {
        return static_cast<foeResourceCreateInfo>(mResReturn);
    }

    foeResult findExternalFile(char const *pExternalFilePath,
                               uint32_t *pPathLength,
                               char *pPath) final {
        if (std::filesystem::exists(std::filesystem::path{FOE_SIMULATION_TEST_DATA_DIR} / mName /
                                    pExternalFilePath)) {
            return foeResult{.value = FOE_IMEX_SUCCESS};
        }

        return foeResult{.value = FOE_IMEX_FILE_NOT_FOUND};
    }

  private:
    std::string mName;
    foeIdGroup mGroup;
    void *mResReturn;
};

TEST_CASE("foeGroupData - Initial State", "[foe]") {
    foeGroupData test;

    SECTION("Persistent Group") {
        SECTION("Resource Indexes") {
            REQUIRE(test.persistentResourceIndexes() != FOE_NULL_HANDLE);
            REQUIRE(test.persistentResourceIndexes() == test.resourceIndexes(foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.persistentResourceIndexes()) ==
                    foeIdPersistentGroup);
            REQUIRE(test.persistentResourceIndexes() == test.resourceIndexes("Persistent"));

            REQUIRE(test.resourceIndexes(foeIdPersistentGroup) != FOE_NULL_HANDLE);
            REQUIRE(test.resourceIndexes(foeIdPersistentGroup) ==
                    test.resourceIndexes(foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.resourceIndexes(foeIdPersistentGroup)) ==
                    foeIdPersistentGroup);
            REQUIRE(test.resourceIndexes(foeIdPersistentGroup) ==
                    test.resourceIndexes("Persistent"));
        }

        SECTION("Entity Indexes") {
            REQUIRE(test.persistentEntityIndexes() != FOE_NULL_HANDLE);
            REQUIRE(test.persistentEntityIndexes() == test.entityIndexes(foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.persistentEntityIndexes()) ==
                    foeIdPersistentGroup);
            REQUIRE(test.persistentEntityIndexes() == test.entityIndexes("Persistent"));

            REQUIRE(test.entityIndexes(foeIdPersistentGroup) != FOE_NULL_HANDLE);
            REQUIRE(test.entityIndexes(foeIdPersistentGroup) ==
                    test.entityIndexes(foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.entityIndexes(foeIdPersistentGroup)) ==
                    foeIdPersistentGroup);
            REQUIRE(test.entityIndexes(foeIdPersistentGroup) == test.entityIndexes("Persistent"));
        }

        SECTION("Importer") {
            REQUIRE(test.persistentImporter() == FOE_NULL_HANDLE);
            REQUIRE(test.persistentImporter() == test.importer(foeIdPersistentGroup));
            REQUIRE(test.persistentImporter() == test.importer("Persistent"));

            REQUIRE(test.importer(foeIdPersistentGroup) == FOE_NULL_HANDLE);
        }
    }

    SECTION("Temporary Group") {
        SECTION("Resource Indexes") {
            REQUIRE(test.temporaryResourceIndexes() != FOE_NULL_HANDLE);
            REQUIRE(test.temporaryResourceIndexes() == test.resourceIndexes(foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.temporaryResourceIndexes()) ==
                    foeIdTemporaryGroup);
            REQUIRE(test.temporaryResourceIndexes() == test.resourceIndexes("Temporary"));

            REQUIRE(test.resourceIndexes(foeIdTemporaryGroup) != FOE_NULL_HANDLE);
            REQUIRE(test.resourceIndexes(foeIdTemporaryGroup) ==
                    test.resourceIndexes(foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.resourceIndexes(foeIdTemporaryGroup)) ==
                    foeIdTemporaryGroup);
            REQUIRE(test.resourceIndexes(foeIdTemporaryGroup) == test.resourceIndexes("Temporary"));
        }

        SECTION("Entity Indexes") {
            REQUIRE(test.temporaryEntityIndexes() != FOE_NULL_HANDLE);
            REQUIRE(test.temporaryEntityIndexes() == test.entityIndexes(foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.temporaryEntityIndexes()) == foeIdTemporaryGroup);
            REQUIRE(test.temporaryEntityIndexes() == test.entityIndexes("Temporary"));

            REQUIRE(test.entityIndexes(foeIdTemporaryGroup) != FOE_NULL_HANDLE);
            REQUIRE(test.entityIndexes(foeIdTemporaryGroup) ==
                    test.entityIndexes(foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(test.entityIndexes(foeIdTemporaryGroup)) ==
                    foeIdTemporaryGroup);
            REQUIRE(test.entityIndexes(foeIdTemporaryGroup) == test.entityIndexes("Temporary"));
        }

        SECTION("Never an importer for the Temporary Group") {
            REQUIRE(test.importer(foeIdTemporaryGroup) == nullptr);
        }
    }

    SECTION("Dynamic Groups are all empty/nullptr") {
        for (uint32_t i = 0; i < (foeIdGroupMaxValue - 2); ++i) {
            REQUIRE(test.resourceIndexes(foeIdValueToGroup(i)) == FOE_NULL_HANDLE);
            REQUIRE(test.entityIndexes(foeIdValueToGroup(i)) == FOE_NULL_HANDLE);
            REQUIRE(test.importer(foeIdValueToGroup(i)) == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - addDynamicGroup failure cases", "[foe]") {
    foeGroupData test;

    SECTION("No EntityIndexes given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
    }

    SECTION("No ResourceIndexes given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("No Importer given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        std::unique_ptr<DummyImporter> testImporter;

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Blank name in the importer") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Inconsistent ID Groups") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Same as 'Persistent' ID Group") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdPersistentGroup, &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdPersistentGroup, &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("a", foeIdPersistentGroup);

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Same as 'Persistent' reserved name") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("Persistent", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Same as 'Temporary' ID Group") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdTemporaryGroup, &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdTemporaryGroup, &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("a", foeIdTemporaryGroup);

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Same as 'Temporary' reserved name") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("Temporary", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Adding the same ID group more than once fails") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        testImporter = std::make_unique<DummyImporter>("b", foeIdValueToGroup(0x1));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("Adding the same Importer group name more than once fails") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

        REQUIRE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x2), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x2), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x2));

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }
}

TEST_CASE("foeGroupData - setPersistentImporter failure cases", "[foe]") {
    foeGroupData test;

    SECTION("Giving a nullptr") { REQUIRE_FALSE(test.setPersistentImporter(nullptr)); }

    SECTION("Giving an importer that is not foeIdPersistentGroup") {
        SECTION("Dynamic Groups") {
            for (foeIdGroup group = 0; group < foeIdNumDynamicGroups; ++group) {
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

TEST_CASE("foeGroupData - setPersistentImporter success cases", "[foe]") {
    foeGroupData test;

    auto tempImporter =
        std::make_unique<DummyImporter>("testPersistentImporter", foeIdPersistentGroup);
    auto *pImporter = tempImporter.get();

    REQUIRE(test.setPersistentImporter(std::move(tempImporter)));

    REQUIRE(test.importer(foeIdPersistentGroup) == pImporter);

    SECTION("Persistent importer with a different importer name also works") {
        REQUIRE(test.importer("testPersistentImporter") == pImporter);
    }
}

TEST_CASE("foeGroupData - foeEcsIndexes/Importer retrieval", "[foe]") {
    foeGroupData test;

    foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
    foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
            FOE_ECS_SUCCESS);
    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
            FOE_ECS_SUCCESS);

    auto testImporter = std::make_unique<DummyImporter>("0x1", foeIdValueToGroup(0x1));
    REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

    SECTION("0x1 Group Data") {
        REQUIRE(test.resourceIndexes(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.resourceIndexes("0x1") != nullptr);

        REQUIRE(test.entityIndexes(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.entityIndexes("0x1") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(0x1)) != nullptr);
        REQUIRE(test.importer("0x1") != nullptr);
    }

    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue), &testEntityIndexes)
                .value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue), &testResourceIndexes)
                .value == FOE_ECS_SUCCESS);

    testImporter =
        std::make_unique<DummyImporter>("MaxDynamic", foeIdValueToGroup(foeIdMaxDynamicGroupValue));
    REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, std::move(testImporter)));

    SECTION("MaxDynamic Group Data") {
        REQUIRE(test.resourceIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.resourceIndexes("MaxDynamic") != nullptr);

        REQUIRE(test.entityIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.entityIndexes("MaxDynamic") != nullptr);

        REQUIRE(test.importer(foeIdValueToGroup(foeIdMaxDynamicGroupValue)) != nullptr);
        REQUIRE(test.importer("MaxDynamic") != nullptr);
    }

    SECTION("Failure cases") {
        SECTION("Temporary importer by name fails") {
            REQUIRE(test.importer("Temporary") == nullptr);
        }

        SECTION("Name not added") {
            REQUIRE(test.resourceIndexes("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.resourceIndexes("foeIdNumDynamicGroups") == nullptr);

            REQUIRE(test.entityIndexes("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.entityIndexes("foeIdNumDynamicGroups") == nullptr);

            REQUIRE(test.importer("foeIdNumDynamicGroups") == nullptr);
            REQUIRE(test.importer("foeIdNumDynamicGroups") == nullptr);
        }
    }
}

TEST_CASE("foeGroupData - getResourceDefinition", "[foe]") {
    foeGroupData test;
    foeResourceCreateInfo createInfo;

    SECTION("No importers always fails") {
        REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
    }

    SECTION("Persistent importer added") {
        SECTION("Returning true") {
            auto persistentImporter = std::make_unique<DummyImporter>(
                "testPersistent", foeIdPersistentGroup, static_cast<int *>(NULL) + 1);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            auto persistentImporter =
                std::make_unique<DummyImporter>("testPersistent", foeIdPersistentGroup);
            REQUIRE(test.setPersistentImporter(std::move(persistentImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
        }
    }

    SECTION("Dynamic importer added") {
        SECTION("Returning true") {
            foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
            foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                    FOE_ECS_SUCCESS);
            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                    FOE_ECS_SUCCESS);

            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1),
                                                                static_cast<int *>(NULL) + 1);

            REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes,
                                         std::move(testImporter)));

            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
            foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                    FOE_ECS_SUCCESS);
            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                    FOE_ECS_SUCCESS);

            auto testImporter = std::make_unique<DummyImporter>("a", foeIdValueToGroup(0x1));

            REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes,
                                         std::move(testImporter)));
            REQUIRE(test.getResourceDefinition(FOE_INVALID_ID) == nullptr);
        }
    }
}
