// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/ecs/error_code.h>
#include <foe/ecs/indexes.h>
#include <foe/ecs/name_map.h>
#include <foe/imex/error_code.h>
#include <foe/imex/yaml/error_code.h>
#include <foe/imex/yaml/importer.hpp>
#include <foe/simulation/simulation.hpp>

#include "../src/importer_registration.hpp"
#include "test_common.hpp"

#include <cstring>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("foeYamlImporter - Function Tests") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    testPath /= "11-good-content";
    foeImporterBase *pTestImporter{nullptr};

    foeResult result = foeImexYamlCreateImporter(2, testPath.string().c_str(), &pTestImporter);

    REQUIRE(result.value == FOE_IMEX_YAML_SUCCESS);
    REQUIRE(pTestImporter != nullptr);

    CHECK(pTestImporter->group() == 2);
    CHECK(std::string_view{pTestImporter->name()} == "11-good-content");

    SECTION("Dependencies (getDependencies)") {
        uint32_t dependenciesCount;
        uint32_t namesLength;

        REQUIRE(pTestImporter->getDependencies(&dependenciesCount, nullptr, &namesLength, nullptr)
                    .value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(dependenciesCount == 2);
        REQUIRE(namesLength == 14);

        SECTION("With correctly sizes buffers") {
            std::array<foeIdGroup, 2> groups;
            std::array<char, 14> names;

            REQUIRE(
                pTestImporter
                    ->getDependencies(&dependenciesCount, groups.data(), &namesLength, names.data())
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

            REQUIRE(
                pTestImporter
                    ->getDependencies(&dependenciesCount, groups.data(), &namesLength, names.data())
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

            REQUIRE(
                pTestImporter
                    ->getDependencies(&dependenciesCount, groups.data(), &namesLength, names.data())
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

            REQUIRE(
                pTestImporter
                    ->getDependencies(&dependenciesCount, groups.data(), &namesLength, names.data())
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

            REQUIRE(
                pTestImporter
                    ->getDependencies(&dependenciesCount, groups.data(), &namesLength, names.data())
                    .value == FOE_IMEX_YAML_INCOMPLETE);
            REQUIRE(dependenciesCount == 2);
            REQUIRE(namesLength == 0);

            CHECK(groups[0] == 0);
        }
    }

    SECTION("Resource Index Data (getGroupResourceIndexData)") {
        foeEcsIndexes indexes{FOE_NULL_HANDLE};

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0), &indexes).value == FOE_ECS_SUCCESS);
        CHECK(indexes != FOE_NULL_HANDLE);

        pTestImporter->getGroupResourceIndexData(indexes);

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

    SECTION("Entity Index Data (getGroupEntityIndexData)") {
        foeEcsIndexes indexes{FOE_NULL_HANDLE};

        REQUIRE(foeEcsCreateIndexes(foeIdValueToGroup(0), &indexes).value == FOE_ECS_SUCCESS);
        CHECK(indexes != FOE_NULL_HANDLE);

        pTestImporter->getGroupEntityIndexData(indexes);

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

    SECTION("Entity State Data (importStateData)") {
        foeSimulation *pTestSimulation{nullptr};

        foeResult result = foeCreateSimulation(true, &pTestSimulation);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(pTestSimulation != nullptr);

        SECTION("With the importer plugin data not registered, the import fails") {
            bool imported =
                pTestImporter->importStateData(pTestSimulation->entityNameMap, pTestSimulation);
            CHECK_FALSE(imported);
        }

        SECTION("With the importer plugin data registered, the import succeeds") {
            REQUIRE(registerTestImporterContent());

            bool imported =
                pTestImporter->importStateData(pTestSimulation->entityNameMap, pTestSimulation);
            CHECK(imported);

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

    SECTION("Finding external data file (findExternalFile)") {
        uint32_t pathLength = UINT32_MAX;
        foeResult result;

        SECTION("Existing file") {
            result = pTestImporter->findExternalFile("findable_external_file", &pathLength, NULL);
            CHECK(result.value == FOE_IMEX_SUCCESS);
            CHECK(pathLength != UINT32_MAX);

            std::string path;
            path.resize(pathLength);
            result =
                pTestImporter->findExternalFile("findable_external_file", &pathLength, path.data());
            CHECK(result.value == FOE_IMEX_SUCCESS);
            CHECK(pathLength == path.size());
            CHECK(path.find("findable_external_file") != std::string::npos);
        }

        SECTION("Non-existing file") {
            result = pTestImporter->findExternalFile("non-existing-file", &pathLength, NULL);
            CHECK(result.value == FOE_IMEX_FILE_NOT_FOUND);
            CHECK(pathLength == UINT32_MAX);
        }
    }

    delete pTestImporter;
}