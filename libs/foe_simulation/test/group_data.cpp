// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/id.h>
#include <foe/ecs/result.h>
#include <foe/imex/result.h>
#include <foe/imex/type_defs.h>
#include <foe/simulation/group_data.h>
#include <foe/simulation/result.h>

#include <string>

namespace {

struct DummyImporterData {
    foeStructureType sType;
    void *pNext;

    std::string name;
    foeIdGroup group;
    void *resReturn;
};

foeResultSet dummyGetGroupID(foeImexImporter importer, foeIdGroup *pGroupID) {
    DummyImporterData *pDummyImporter = (DummyImporterData *)importer;

    *pGroupID = pDummyImporter->group;
    return foeResultSet{};
}

foeResultSet dummyGetGroupName(foeImexImporter importer, char const **ppGroupName) {
    DummyImporterData *pDummyImporter = (DummyImporterData *)importer;

    *ppGroupName = pDummyImporter->name.c_str();
    return foeResultSet{};
}

foeResultSet dummyGetResourceCreateInfo(foeImexImporter importer,
                                        foeId id,
                                        foeResourceCreateInfo *pCI) {
    DummyImporterData *pDummyImporter = (DummyImporterData *)importer;

    *pCI = static_cast<foeResourceCreateInfo>(pDummyImporter->resReturn);
    return foeResultSet{};
}

foeImexImporterCalls dummyImporterCalls{
    .sType = FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS,
    .getGroupID = dummyGetGroupID,
    .getGroupName = dummyGetGroupName,
    .getResourceCreateInfo = dummyGetResourceCreateInfo,
};

} // namespace

TEST_CASE("foeGroupData - Initial State", "[foe]") {
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("Persistent Group") {
        SECTION("Resource Indexes") {
            REQUIRE(foeSimulationPersistentResourceIndexes(test) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationPersistentResourceIndexes(test) ==
                    foeSimulationResourceIndexes(test, foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationPersistentResourceIndexes(test)) ==
                    foeIdPersistentGroup);

            REQUIRE(foeSimulationResourceIndexes(test, foeIdPersistentGroup) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationResourceIndexes(test, foeIdPersistentGroup) ==
                    foeSimulationResourceIndexes(test, foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationResourceIndexes(
                        test, foeIdPersistentGroup)) == foeIdPersistentGroup);
        }

        SECTION("Entity Indexes") {
            REQUIRE(foeSimulationPersistentEntityIndexes(test) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationPersistentEntityIndexes(test) ==
                    foeSimulationEntityIndexes(test, foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationPersistentEntityIndexes(test)) ==
                    foeIdPersistentGroup);

            REQUIRE(foeSimulationEntityIndexes(test, foeIdPersistentGroup) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationEntityIndexes(test, foeIdPersistentGroup) ==
                    foeSimulationEntityIndexes(test, foeIdPersistentGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationEntityIndexes(
                        test, foeIdPersistentGroup)) == foeIdPersistentGroup);
        }

        SECTION("Importer") {
            REQUIRE(foeSimulationPersistentImporter(test) == FOE_NULL_HANDLE);
            REQUIRE(foeSimulationPersistentImporter(test) ==
                    foeSimulationImporter(test, foeIdPersistentGroup));

            REQUIRE(foeSimulationImporter(test, foeIdPersistentGroup) == FOE_NULL_HANDLE);
        }
    }

    SECTION("Temporary Group") {
        SECTION("Resource Indexes") {
            REQUIRE(foeSimulationTemporaryResourceIndexes(test) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationTemporaryResourceIndexes(test) ==
                    foeSimulationResourceIndexes(test, foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationTemporaryResourceIndexes(test)) ==
                    foeIdTemporaryGroup);

            REQUIRE(foeSimulationResourceIndexes(test, foeIdTemporaryGroup) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationResourceIndexes(test, foeIdTemporaryGroup) ==
                    foeSimulationResourceIndexes(test, foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationResourceIndexes(
                        test, foeIdTemporaryGroup)) == foeIdTemporaryGroup);
        }

        SECTION("Entity Indexes") {
            REQUIRE(foeSimulationTemporaryEntityIndexes(test) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationTemporaryEntityIndexes(test) ==
                    foeSimulationEntityIndexes(test, foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationTemporaryEntityIndexes(test)) ==
                    foeIdTemporaryGroup);

            REQUIRE(foeSimulationEntityIndexes(test, foeIdTemporaryGroup) != FOE_NULL_HANDLE);
            REQUIRE(foeSimulationEntityIndexes(test, foeIdTemporaryGroup) ==
                    foeSimulationEntityIndexes(test, foeIdTemporaryGroup));
            REQUIRE(foeEcsIndexesGetGroupID(foeSimulationEntityIndexes(
                        test, foeIdTemporaryGroup)) == foeIdTemporaryGroup);
        }

        SECTION("Never an importer for the Temporary Group") {
            REQUIRE(foeSimulationImporter(test, foeIdTemporaryGroup) == nullptr);
        }
    }

    SECTION("Dynamic Groups are all empty/nullptr") {
        for (uint32_t i = 0; i < (foeIdGroupMaxValue - 2); ++i) {
            REQUIRE(foeSimulationResourceIndexes(test, foeIdValueToGroup(i)) == FOE_NULL_HANDLE);
            REQUIRE(foeSimulationEntityIndexes(test, foeIdValueToGroup(i)) == FOE_NULL_HANDLE);
            REQUIRE(foeSimulationImporter(test, foeIdValueToGroup(i)) == nullptr);
        }
    }

    foeSimulationDestroyGroupData(test);
}

TEST_CASE("foeGroupData - addDynamicGroup failure cases", "[foe]") {
    DummyImporterData dummyImporter, dummyImporter2;
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("No EntityIndexes given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

        foeEcsDestroyIndexes(testResourceIndexes);
    }

    SECTION("No ResourceIndexes given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("No Importer given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   FOE_NULL_HANDLE));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   std::move(testImporter)));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x2),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdPersistentGroup,
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "Persistent",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdTemporaryGroup,
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "Temporary",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                             testImporter));

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        dummyImporter2 = {
            .pNext = &dummyImporterCalls,
            .name = "test2",
            .group = foeIdValueToGroup(0x1),
        };
        testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter2);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x1),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                             testImporter));

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x2), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x2), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        dummyImporter2 = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x2),
        };
        testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter2);

        REQUIRE_FALSE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                   testImporter));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }

    foeSimulationDestroyGroupData(test);
}

TEST_CASE("foeGroupData - setPersistentImporter failure cases", "[foe]") {
    DummyImporterData dummyImporter;
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("Giving a nullptr") {
        REQUIRE_FALSE(foeSimulationSetPersistentImporter(test, nullptr));
    }

    SECTION("Giving an importer that is not foeIdPersistentGroup") {
        SECTION("Dynamic Groups") {
            for (foeIdGroup group = 0; group < foeIdNumDynamicGroups; ++group) {
                dummyImporter = {
                    .pNext = &dummyImporterCalls,
                    .name = "test",
                    .group = group,
                };

                REQUIRE_FALSE(foeSimulationSetPersistentImporter(test, testImporter));
            }
        }

        SECTION("foeIdTemporaryGroup") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "test",
                .group = foeIdTemporaryGroup,
            };

            REQUIRE_FALSE(foeSimulationSetPersistentImporter(test, testImporter));
        }
    }

    foeSimulationDestroyGroupData(test);
}

TEST_CASE("foeGroupData - setPersistentImporter success cases", "[foe]") {
    DummyImporterData dummyImporter = {
        .pNext = &dummyImporterCalls,
        .name = "testPersistentImporter",
        .group = foeIdPersistentGroup,
    };
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    REQUIRE(foeSimulationSetPersistentImporter(test, testImporter));

    REQUIRE(foeSimulationImporter(test, foeIdPersistentGroup) == testImporter);

    foeSimulationDestroyGroupData(test);
}

TEST_CASE("foeGroupData - foeEcsIndexes/Importer retrieval", "[foe]") {
    DummyImporterData dummyImporter, dummyImporter2;
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
    foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
            FOE_ECS_SUCCESS);
    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
            FOE_ECS_SUCCESS);

    dummyImporter = {
        .pNext = &dummyImporterCalls,
        .name = "0x1",
        .group = foeIdValueToGroup(0x1),
    };
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

    REQUIRE(
        foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes, testImporter));

    SECTION("0x1 Group Data") {
        REQUIRE(foeSimulationResourceIndexes(test, foeIdValueToGroup(0x1)) != nullptr);

        REQUIRE(foeSimulationEntityIndexes(test, foeIdValueToGroup(0x1)) != nullptr);

        REQUIRE(foeSimulationImporter(test, foeIdValueToGroup(0x1)) != nullptr);
    }

    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue), &testEntityIndexes)
                .value == FOE_ECS_SUCCESS);
    REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(foeIdMaxDynamicGroupValue), &testResourceIndexes)
                .value == FOE_ECS_SUCCESS);

    dummyImporter2 = {
        .pNext = &dummyImporterCalls,
        .name = "MaxDynamic",
        .group = foeIdValueToGroup(foeIdMaxDynamicGroupValue),
    };
    testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter2);

    REQUIRE(
        foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes, testImporter));

    SECTION("MaxDynamic Group Data") {
        REQUIRE(foeSimulationResourceIndexes(test, foeIdValueToGroup(foeIdMaxDynamicGroupValue)) !=
                nullptr);

        REQUIRE(foeSimulationEntityIndexes(test, foeIdValueToGroup(foeIdMaxDynamicGroupValue)) !=
                nullptr);

        REQUIRE(foeSimulationImporter(test, foeIdValueToGroup(foeIdMaxDynamicGroupValue)) !=
                nullptr);
    }

    foeSimulationDestroyGroupData(test);
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("pointer-overflow"))) // pointer arithmetic with nullptr
#endif
TEST_CASE("foeGroupData - getResourceCreateInfo", "[foe]") {
    DummyImporterData dummyImporter;
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);
    foeGroupData test = FOE_NULL_HANDLE;
    foeResultSet result;

    result = foeSimulationCreateGroupData(&test);
    REQUIRE(result.value == FOE_SIMULATION_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("No importers always fails") {
        REQUIRE(foeSimulationGetGroupDataResourceCreateInfo(test, FOE_INVALID_ID) == nullptr);
    }

    SECTION("Persistent importer added") {
        SECTION("Returning true") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "testPersistent",
                .group = foeIdPersistentGroup,
                .resReturn = static_cast<int *>(NULL) + 1,
            };

            REQUIRE(foeSimulationSetPersistentImporter(test, testImporter));

            REQUIRE(foeSimulationGetGroupDataResourceCreateInfo(test, FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "testPersistent",
                .group = foeIdPersistentGroup,
            };

            REQUIRE(foeSimulationSetPersistentImporter(test, testImporter));

            REQUIRE(foeSimulationGetGroupDataResourceCreateInfo(test, FOE_INVALID_ID) == nullptr);
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

            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "0x1",
                .group = foeIdValueToGroup(0x1),
                .resReturn = static_cast<int *>(NULL) + 1,
            };

            REQUIRE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                 testImporter));

            REQUIRE(foeSimulationGetGroupDataResourceCreateInfo(test, FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
            foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                    FOE_ECS_SUCCESS);
            REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                    FOE_ECS_SUCCESS);

            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "0x1",
                .group = foeIdValueToGroup(0x1),
            };

            REQUIRE(foeSimulationAddDynamicGroup(test, testEntityIndexes, testResourceIndexes,
                                                 testImporter));
            REQUIRE(foeSimulationGetGroupDataResourceCreateInfo(test, FOE_INVALID_ID) == nullptr);
        }
    }

    foeSimulationDestroyGroupData(test);
}
