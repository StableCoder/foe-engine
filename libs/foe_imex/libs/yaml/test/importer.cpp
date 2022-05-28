/*
    Copyright (C) 2022 George Cave.

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
#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/error_code.h>
#include <foe/ecs/index_generator.hpp>
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
    CHECK(pTestImporter->name() == "11-good-content");

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
        foeIdIndexGenerator generator{foeIdValueToGroup(0)};
        pTestImporter->getGroupResourceIndexData(generator);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;

        uint32_t count;
        REQUIRE(generator.exportState(nullptr, &count, nullptr).value == FOE_ECS_SUCCESS);
        CHECK(count == 2);

        recycled.resize(count);
        REQUIRE(generator.exportState(&nextFreshIndex, &count, recycled.data()).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(count == 2);

        CHECK(nextFreshIndex == 4);
        CHECK(recycled[0] == 3);
        CHECK(recycled[1] == 1);
    }

    SECTION("Entity Index Data (getGroupEntityIndexData)") {
        foeIdIndexGenerator generator{foeIdValueToGroup(0)};
        pTestImporter->getGroupEntityIndexData(generator);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;

        uint32_t count;
        REQUIRE(generator.exportState(nullptr, &count, nullptr).value == FOE_ECS_SUCCESS);
        CHECK(count == 2);

        recycled.resize(count);
        REQUIRE(generator.exportState(&nextFreshIndex, &count, recycled.data()).value ==
                FOE_ECS_SUCCESS);
        REQUIRE(count == 2);

        CHECK(nextFreshIndex == 3);
        CHECK(recycled[0] == 0);
        CHECK(recycled[1] == 2);
    }

    SECTION("Entity State Data (importStateData)") {
        foeSimulation *pTestSimulation{nullptr};

        foeResult result = foeCreateSimulation(true, &pTestSimulation);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(pTestSimulation != nullptr);

        SECTION("With the importer plugin data not registered, the import fails") {
            bool imported =
                pTestImporter->importStateData(pTestSimulation->pEntityNameMap, pTestSimulation);
            CHECK_FALSE(imported);
        }

        SECTION("With the importer plugin data registered, the import succeeds") {
            REQUIRE(registerTestImporterContent());

            bool imported =
                pTestImporter->importStateData(pTestSimulation->pEntityNameMap, pTestSimulation);
            CHECK(imported);

            CHECK(pTestSimulation->pEntityNameMap->find(foeIdPersistentGroup | 0x2) ==
                  "Entity-0x2");
            CHECK(pTestSimulation->pEntityNameMap->find("Entity-0x2") ==
                  (foeIdPersistentGroup | 0x2));

            deregisterTestImporterContent();
        }

        foeDestroySimulation(pTestSimulation);
    }

    SECTION("Finding external data file (findExternalFile)") {
        std::filesystem::path externalPath =
            pTestImporter->findExternalFile("findable_external_file");

        CHECK_FALSE(externalPath.empty());

        externalPath = pTestImporter->findExternalFile("non-existing-file");

        CHECK(externalPath.empty());
    }

    delete pTestImporter;
}