// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/ecs/indexes.h>
#include <foe/ecs/name_map.h>
#include <foe/ecs/result.h>
#include <foe/imex/result.h>
#include <foe/imex/yaml/importer.hpp>
#include <foe/imex/yaml/result.h>
#include <foe/simulation/simulation.hpp>

#include "test_common.hpp"

#include <cstring>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("foeYamlImporter - Function Tests") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    testPath /= "11-good-content";
    foeImexImporter testImporter{FOE_NULL_HANDLE};

    foeResultSet result = foeCreateYamlImporter(2, testPath.string().c_str(), &testImporter);

    REQUIRE(result.value == FOE_IMEX_YAML_SUCCESS);
    REQUIRE(testImporter != FOE_NULL_HANDLE);

    foeIdGroup groupID = FOE_INVALID_ID;
    result = foeImexImporterGetGroupID(testImporter, &groupID);

    CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
    CHECK(groupID == 2);

    char const *pGroupName = nullptr;
    result = foeImexImporterGetGroupName(testImporter, &pGroupName);

    CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
    REQUIRE(pGroupName != nullptr);
    CHECK(std::string_view{pGroupName} == "11-good-content");

    SECTION("Dependencies (getDependencies)") {
        uint32_t dependenciesCount;
        uint32_t namesLength;

        REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, nullptr,
                                               &namesLength, nullptr)
                    .value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(dependenciesCount == 2);
        REQUIRE(namesLength == 14);

        SECTION("With correctly sizes buffers") {
            std::array<foeIdGroup, 2> groups;
            std::array<char, 14> names;

            REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, groups.data(),
                                                   &namesLength, names.data())
                        .value == FOE_IMEX_YAML_SUCCESS);
            REQUIRE(dependenciesCount == 2);
            REQUIRE(namesLength == 14);

            CHECK(groups[0] == 0);
            CHECK(std::string_view{&names[0]} == "test01");

            CHECK(groups[1] == foeIdValueToGroup(1));
            CHECK(std::string_view{&names[7]} == "test02");
        }
        SECTION("With oversized buffers") {
            std::array<foeIdGroup, 3> groups;
            dependenciesCount = 3;
            std::array<char, 24> names;
            namesLength = 24;

            REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, groups.data(),
                                                   &namesLength, names.data())
                        .value == FOE_IMEX_YAML_SUCCESS);
            REQUIRE(dependenciesCount == 2);
            REQUIRE(namesLength == 14);

            CHECK(groups[0] == 0);
            CHECK(std::string_view{&names[0]} == "test01");

            CHECK(groups[1] == foeIdValueToGroup(1));
            CHECK(std::string_view{&names[7]} == "test02");
        }
        SECTION("With undersized GroupID buffers") {
            std::array<foeIdGroup, 1> groups;
            dependenciesCount = 1;
            std::array<char, 14> names;

            REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, groups.data(),
                                                   &namesLength, names.data())
                        .value == FOE_IMEX_YAML_INCOMPLETE);
            REQUIRE(dependenciesCount == 1);
            REQUIRE(namesLength == 14);

            CHECK(groups[0] == 0);
            CHECK(std::string_view{&names[0]} == "test01");
        }
        SECTION("With undersized name buffer") {
            std::array<foeIdGroup, 2> groups;
            std::array<char, 10> names;
            namesLength = 10;

            REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, groups.data(),
                                                   &namesLength, names.data())
                        .value == FOE_IMEX_YAML_INCOMPLETE);
            REQUIRE(dependenciesCount == 2);
            REQUIRE(namesLength == 10);

            CHECK(groups[0] == 0);
            CHECK(strcmp(&names[0], "test01") == 0);

            CHECK(groups[1] == foeIdValueToGroup(1));
            CHECK(strncmp(&names[7], "te", 3) == 0);
        }
        SECTION("With zero-sized name buffer") {
            std::array<foeIdGroup, 2> groups;
            std::array<char, 0> names;
            namesLength = 0;

            REQUIRE(foeImexImporterGetDependencies(testImporter, &dependenciesCount, groups.data(),
                                                   &namesLength, names.data())
                        .value == FOE_IMEX_YAML_INCOMPLETE);
            REQUIRE(dependenciesCount == 2);
            REQUIRE(namesLength == 0);

            CHECK(groups[0] == 0);
        }
    }

    SECTION("Resource Index Data (foeImexImporterGetGroupResourceIndexData)") {
        foeEcsIndexes indexes{FOE_NULL_HANDLE};

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0), &indexes).value == FOE_ECS_SUCCESS);
        CHECK(indexes != FOE_NULL_HANDLE);

        foeImexImporterGetGroupResourceIndexData(testImporter, indexes);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;

        uint32_t count;
        REQUIRE(foeEcsExportIndexes(indexes, nullptr, &count, nullptr).value == FOE_ECS_SUCCESS);
        CHECK(count == 2);

        recycled.resize(count);
        REQUIRE(foeEcsExportIndexes(indexes, &nextFreshIndex, &count, recycled.data()).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(count == 2);

        CHECK(nextFreshIndex == 4);
        CHECK(recycled[0] == 3);
        CHECK(recycled[1] == 1);

        foeEcsDestroyIndexes(indexes);
    }

    SECTION("Entity Index Data (foeImexImporterGetGroupEntityIndexData)") {
        foeEcsIndexes indexes{FOE_NULL_HANDLE};

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0), &indexes).value == FOE_ECS_SUCCESS);
        CHECK(indexes != FOE_NULL_HANDLE);

        foeImexImporterGetGroupEntityIndexData(testImporter, indexes);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;

        uint32_t count;
        REQUIRE(foeEcsExportIndexes(indexes, nullptr, &count, nullptr).value == FOE_ECS_SUCCESS);
        CHECK(count == 2);

        recycled.resize(count);
        REQUIRE(foeEcsExportIndexes(indexes, &nextFreshIndex, &count, recycled.data()).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(count == 2);

        CHECK(nextFreshIndex == 3);
        CHECK(recycled[0] == 0);
        CHECK(recycled[1] == 2);

        foeEcsDestroyIndexes(indexes);
    }

    SECTION("Entity State Data (foeImexImporterGetStateData)") {
        foeSimulation *pTestSimulation{nullptr};

        foeResultSet result = foeCreateSimulation(true, &pTestSimulation);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(pTestSimulation != nullptr);

        SECTION("With the importer plugin data not registered, the import fails") {
            result = foeImexImporterGetStateData(testImporter, pTestSimulation->entityNameMap,
                                                 pTestSimulation);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_FAILED_TO_FIND_COMPONENT_IMPORTER);
        }

        SECTION("With the importer plugin data registered, the import succeeds") {
            REQUIRE(registerTestImporterContent());

            result = foeImexImporterGetStateData(testImporter, pTestSimulation->entityNameMap,
                                                 pTestSimulation);
            CHECK(result.value == FOE_SUCCESS);

            foeId id;
            CHECK(foeEcsNameMapFindID(pTestSimulation->entityNameMap, "Entity-0x2", &id).value ==
                  FOE_ECS_SUCCESS);
            REQUIRE(id == (foeIdPersistentGroup | 0x2));

            uint32_t strLength = 15;
            char cmpStr[15];
            CHECK(foeEcsNameMapFindName(pTestSimulation->entityNameMap, foeIdPersistentGroup | 0x2,
                                        &strLength, cmpStr)
                      .value == FOE_ECS_SUCCESS);
            CHECK(strLength == 11);
            CHECK(memcmp(cmpStr, "Entity-0x2", 11) == 0);

            deregisterTestImporterContent();
        }

        foeDestroySimulation(pTestSimulation);
    }

    SECTION("Finding external data file (foeImexImporterFindExternalFile)") {
        uint32_t pathLength = UINT32_MAX;
        foeResultSet result;

        SECTION("Existing file") {
            result = foeImexImporterFindExternalFile(testImporter, "findable_external_file",
                                                     &pathLength, NULL);
            CHECK(result.value == FOE_IMEX_SUCCESS);
            CHECK(pathLength != UINT32_MAX);

            std::string path;
            path.resize(pathLength);
            result = foeImexImporterFindExternalFile(testImporter, "findable_external_file",
                                                     &pathLength, path.data());
            CHECK(result.value == FOE_IMEX_SUCCESS);
            CHECK(pathLength == path.size());
            CHECK(path.find("findable_external_file") != std::string::npos);
        }

        SECTION("Non-existing file") {
            result = foeImexImporterFindExternalFile(testImporter, "non-existing-file", &pathLength,
                                                     NULL);
            CHECK(result.value != FOE_SUCCESS);
            CHECK(pathLength == UINT32_MAX);
        }
    }

    foeDestroyImporter(testImporter);
}