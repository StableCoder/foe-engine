// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/ecs/id.h>
#include <foe/ecs/result.h>
#include <foe/imex/result.h>
#include <foe/imex/type_defs.h>
#include <foe/simulation/group_data.hpp>

#include <filesystem>
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
    DummyImporterData dummyImporter, dummyImporter2;
    foeGroupData test;

    /*
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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

        foeEcsDestroyIndexes(testEntityIndexes);
    }

    SECTION("No Importer given") {
        foeEcsIndexes testEntityIndexes = FOE_NULL_HANDLE;
        foeEcsIndexes testResourceIndexes = FOE_NULL_HANDLE;

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testEntityIndexes).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0x1), &testResourceIndexes).value ==
                FOE_ECS_SUCCESS);

        REQUIRE_FALSE(
            test.addDynamicGroup(testEntityIndexes, testResourceIndexes, FOE_NULL_HANDLE));

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

        dummyImporter = {
            .pNext = &dummyImporterCalls,
            .name = "test",
            .group = foeIdValueToGroup(0x2),
        };
        foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

        REQUIRE_FALSE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

        foeEcsDestroyIndexes(testResourceIndexes);
        foeEcsDestroyIndexes(testEntityIndexes);
    }
    */
}

TEST_CASE("foeGroupData - setPersistentImporter failure cases", "[foe]") {
    DummyImporterData dummyImporter;
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);
    foeGroupData test;

    SECTION("Giving a nullptr") { REQUIRE_FALSE(test.setPersistentImporter(nullptr)); }

    SECTION("Giving an importer that is not foeIdPersistentGroup") {
        SECTION("Dynamic Groups") {
            for (foeIdGroup group = 0; group < foeIdNumDynamicGroups; ++group) {
                dummyImporter = {
                    .pNext = &dummyImporterCalls,
                    .name = "test",
                    .group = group,
                };

                REQUIRE_FALSE(test.setPersistentImporter(testImporter));
            }
        }

        SECTION("foeIdTemporaryGroup") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "test",
                .group = foeIdTemporaryGroup,
            };

            REQUIRE_FALSE(test.setPersistentImporter(testImporter));
        }
    }
}

TEST_CASE("foeGroupData - setPersistentImporter success cases", "[foe]") {
    DummyImporterData dummyImporter = {
        .pNext = &dummyImporterCalls,
        .name = "testPersistentImporter",
        .group = foeIdPersistentGroup,
    };
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);
    foeGroupData test;

    REQUIRE(test.setPersistentImporter(testImporter));

    REQUIRE(test.importer(foeIdPersistentGroup) == testImporter);

    SECTION("Persistent importer with a different importer name also works") {
        REQUIRE(test.importer("testPersistentImporter") == testImporter);
    }
}

TEST_CASE("foeGroupData - foeEcsIndexes/Importer retrieval", "[foe]") {
    DummyImporterData dummyImporter, dummyImporter2;
    foeGroupData test;

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
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

    REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

    dummyImporter2 = {
        .pNext = &dummyImporterCalls,
        .name = "MaxDynamic",
        .group = foeIdValueToGroup(foeIdMaxDynamicGroupValue),
    };
    testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter2);

    REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

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

TEST_CASE("foeGroupData - getResourceCreateInfo", "[foe]") {
    DummyImporterData dummyImporter;
    foeImexImporter testImporter = reinterpret_cast<foeImexImporter>(&dummyImporter);

    foeGroupData test;

    SECTION("No importers always fails") {
        REQUIRE(test.getResourceCreateInfo(FOE_INVALID_ID) == nullptr);
    }

    SECTION("Persistent importer added") {
        SECTION("Returning true") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "testPersistent",
                .group = foeIdPersistentGroup,
                .resReturn = static_cast<int *>(NULL) + 1,
            };

            REQUIRE(test.setPersistentImporter(testImporter));

            REQUIRE(test.getResourceCreateInfo(FOE_INVALID_ID) != nullptr);
        }
        SECTION("Returning false") {
            dummyImporter = {
                .pNext = &dummyImporterCalls,
                .name = "testPersistent",
                .group = foeIdPersistentGroup,
            };

            REQUIRE(test.setPersistentImporter(testImporter));

            REQUIRE(test.getResourceCreateInfo(FOE_INVALID_ID) == nullptr);
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

            REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));

            REQUIRE(test.getResourceCreateInfo(FOE_INVALID_ID) != nullptr);
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

            REQUIRE(test.addDynamicGroup(testEntityIndexes, testResourceIndexes, testImporter));
            REQUIRE(test.getResourceCreateInfo(FOE_INVALID_ID) == nullptr);
        }
    }
}
