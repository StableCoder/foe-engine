// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/imex/yaml/error_code.h>
#include <foe/imex/yaml/importer.hpp>

#include "../src/importer_registration.hpp"

#include <filesystem>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("foeImexYamlCreateImporter - Success Case") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    foeImporterBase *pTestImporter{nullptr};

    SECTION("Good layout/files, but empty/no content") {
        testPath /= "10-good-empty";
        foeResult result = foeImexYamlCreateImporter(1, testPath.string().c_str(), &pTestImporter);

        REQUIRE(result.value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(pTestImporter != nullptr);

        CHECK(pTestImporter->group() == 1);
        CHECK(std::string_view{pTestImporter->name()} == "10-good-empty");
    }

    SECTION("Good layout/files, with valid content") {
        testPath /= "11-good-content";
        foeResult result = foeImexYamlCreateImporter(2, testPath.string().c_str(), &pTestImporter);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        CHECK(pTestImporter != nullptr);

        CHECK(pTestImporter->group() == 2);
        CHECK(std::string_view{pTestImporter->name()} == "11-good-content");
    }

    delete pTestImporter;
}

TEST_CASE("foeImexYamlCreateImporter - Failure Cases") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    foeImporterBase *pTestImporter{nullptr};

    SECTION("Root Directory doesn't exist") {
        testPath /= "this_does_no_exist/for_real";
        foeResult result = foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

        CHECK(pTestImporter == nullptr);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY);
    }

    SECTION("Dependencies File") {
        SECTION("Doesn't exist") {
            testPath /= "01-missing-dependencies-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "02-incorrect-dependencies-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Resource Index Data File") {
        SECTION("Doesn't exist") {
            testPath /= "03-missing-resource-index-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "04-incorrect-resource-index-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Entity Index Data File") {
        SECTION("Doesn't exist") {
            testPath /= "05-missing-entity-index-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "06-incorrect-entity-index-file";
            foeResult result =
                foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

            CHECK(pTestImporter == nullptr);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Resources directory not a directory") {
        testPath /= "07-bad-resources-dir";
        foeResult result = foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

        CHECK(pTestImporter == nullptr);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY);
    }

    SECTION("Entities directory not a directory") {
        testPath /= "08-bad-entities-dir";
        foeResult result = foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

        CHECK(pTestImporter == nullptr);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY);
    }

    SECTION("External directory not a directory") {
        testPath /= "09-bad-external-dir";
        foeResult result = foeImexYamlCreateImporter(0, testPath.string().c_str(), &pTestImporter);

        CHECK(pTestImporter == nullptr);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY);
    }
}