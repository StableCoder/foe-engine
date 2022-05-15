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
#include <foe/ecs/index_generator.hpp>
#include <foe/imex/yaml/error_code.h>
#include <foe/imex/yaml/importer.hpp>
#include <foe/simulation/simulation.hpp>

#include "../src/importer_registration.hpp"
#include "test_common.hpp"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

struct foeIdGroupValueNameSet {
    foeIdGroupValue groupValue;
    std::string name;
};

TEST_CASE("foeYamlImporter - Function Tests") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    testPath /= "11-good-content";
    foeImporterBase *pTestImporter{nullptr};

    std::error_code errC = foeImexYamlCreateImporter(2, testPath.string().c_str(), &pTestImporter);

    REQUIRE(errC.value() == FOE_IMEX_YAML_SUCCESS);
    REQUIRE(pTestImporter != nullptr);

    CHECK(pTestImporter->group() == 2);
    CHECK(pTestImporter->name() == "11-good-content");

    SECTION("Dependencies (getDependencies)") {
        std::vector<foeIdGroupValueNameSet> dependencies;

        pTestImporter->getDependencies(dependencies);

        REQUIRE(dependencies.size() == 2);

        CHECK(dependencies[0].name == "test01");
        CHECK(dependencies[0].groupValue == 0);

        CHECK(dependencies[1].name == "test02");
        CHECK(dependencies[1].groupValue == 1);
    }

    SECTION("Resource Index Data (getGroupResourceIndexData)") {
        foeIdIndexGenerator generator{foeIdValueToGroup(0)};
        pTestImporter->getGroupResourceIndexData(generator);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;
        generator.exportState(nextFreshIndex, recycled);

        CHECK(nextFreshIndex == 4);
        REQUIRE(recycled.size() == 2);
        CHECK(recycled[0] == 3);
        CHECK(recycled[1] == 1);
    }

    SECTION("Entity Index Data (getGroupEntityIndexData)") {
        foeIdIndexGenerator generator{foeIdValueToGroup(0)};
        pTestImporter->getGroupEntityIndexData(generator);

        foeIdIndex nextFreshIndex;
        std::vector<foeIdIndex> recycled;
        generator.exportState(nextFreshIndex, recycled);

        CHECK(nextFreshIndex == 3);
        REQUIRE(recycled.size() == 2);
        CHECK(recycled[0] == 0);
        CHECK(recycled[1] == 2);
    }

    SECTION("Entity State Data (importStateData)") {
        foeSimulation *pTestSimulation{nullptr};

        std::error_code errC = foeCreateSimulation(true, &pTestSimulation);
        REQUIRE_FALSE(errC);
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