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
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/imex/yaml/importer_registration.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("Imex De/registration") {
    foeImporterBase *pTestImporter{nullptr};
    std::filesystem::path testPath{TEST_DATA_DIR};
    testPath /= "11-good-content";

    SECTION("Without being registered, Imex doesn't create importer") {
        pTestImporter = createImporter(123, testPath);
        REQUIRE(pTestImporter == nullptr);
    }

    SECTION("When registered, Imex creates the importer") {
        foeImexYamlRegisterImporter();

        pTestImporter = createImporter(123, testPath);
        REQUIRE(pTestImporter != nullptr);
        delete pTestImporter;

        foeImexYamlDeregisterImporter();

        SECTION("After being deregistered, Imex doesn't create importer") {
            pTestImporter = createImporter(123, testPath);
            CHECK(pTestImporter == nullptr);
        }
    }
}