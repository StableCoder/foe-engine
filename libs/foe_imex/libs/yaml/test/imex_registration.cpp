// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/imex/importer.h>
#include <foe/imex/yaml/importer.hpp>
#include <foe/imex/yaml/importer_registration.h>

#include <filesystem>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("Imex De/registration") {
    foeImexImporter testImporter{FOE_NULL_HANDLE};
    std::filesystem::path testPath{TEST_DATA_DIR};
    testPath /= "11-good-content";

    SECTION("Without being registered, Imex doesn't create importer") {
        testImporter = createImporter(123, testPath.string().c_str());
        REQUIRE(testImporter == FOE_NULL_HANDLE);
    }

    SECTION("When registered, Imex creates the importer") {
        foeImexYamlRegisterImporter();

        testImporter = createImporter(123, testPath.string().c_str());
        REQUIRE(testImporter != FOE_NULL_HANDLE);
        foeDestroyImporter(testImporter);

        foeImexYamlDeregisterImporter();

        SECTION("After being deregistered, Imex doesn't create importer") {
            testImporter = createImporter(123, testPath.string().c_str());
            CHECK(testImporter == FOE_NULL_HANDLE);
        }
    }
}