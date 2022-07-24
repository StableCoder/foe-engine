// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/imex/yaml/result.h>

#include <filesystem>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR nullptr
#endif

static_assert(TEST_DATA_DIR != nullptr, "TEST_DATA_DIR must be added as a compilation definition.");

TEST_CASE("foeCreateYamlImporter - Success Case") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    foeImexImporter testImporter{FOE_NULL_HANDLE};

    SECTION("Good layout/files, but empty/no content") {
        testPath /= "10-good-empty";
        foeResultSet result = foeCreateYamlImporter(1, testPath.string().c_str(), &testImporter);

        REQUIRE(result.value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(testImporter != FOE_NULL_HANDLE);

        foeIdGroup groupID = FOE_INVALID_ID;
        result = foeImexImporterGetGroupID(testImporter, &groupID);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        CHECK(groupID == 1);

        char const *pGroupName = nullptr;
        result = foeImexImporterGetGroupName(testImporter, &pGroupName);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(pGroupName != nullptr);
        CHECK(std::string_view{pGroupName} == "10-good-empty");
    }

    SECTION("Good layout/files, with valid content") {
        testPath /= "11-good-content";
        foeResultSet result = foeCreateYamlImporter(2, testPath.string().c_str(), &testImporter);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        CHECK(testImporter != FOE_NULL_HANDLE);

        foeIdGroup groupID = FOE_INVALID_ID;
        result = foeImexImporterGetGroupID(testImporter, &groupID);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        CHECK(groupID == 2);

        char const *pGroupName = nullptr;
        result = foeImexImporterGetGroupName(testImporter, &pGroupName);

        CHECK(result.value == FOE_IMEX_YAML_SUCCESS);
        REQUIRE(pGroupName != nullptr);
        CHECK(std::string_view{pGroupName} == "11-good-content");
    }

    foeDestroyImporter(testImporter);
}

TEST_CASE("foeCreateYamlImporter - Failure Cases") {
    std::filesystem::path testPath{TEST_DATA_DIR};
    foeImexImporter testImporter{FOE_NULL_HANDLE};

    SECTION("Root Directory doesn't exist") {
        testPath /= "this_does_no_exist/for_real";
        foeResultSet result = foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

        CHECK(testImporter == FOE_NULL_HANDLE);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY);
    }

    SECTION("Dependencies File") {
        SECTION("Doesn't exist") {
            testPath /= "01-missing-dependencies-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "02-incorrect-dependencies-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Resource Index Data File") {
        SECTION("Doesn't exist") {
            testPath /= "03-missing-resource-index-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "04-incorrect-resource-index-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Entity Index Data File") {
        SECTION("Doesn't exist") {
            testPath /= "05-missing-entity-index-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST);
        }

        SECTION("Not a regular file") {
            testPath /= "06-incorrect-entity-index-file";
            foeResultSet result =
                foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

            CHECK(testImporter == FOE_NULL_HANDLE);
            CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE);
        }
    }

    SECTION("Resources directory not a directory") {
        testPath /= "07-bad-resources-dir";
        foeResultSet result = foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

        CHECK(testImporter == FOE_NULL_HANDLE);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY);
    }

    SECTION("Entities directory not a directory") {
        testPath /= "08-bad-entities-dir";
        foeResultSet result = foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

        CHECK(testImporter == FOE_NULL_HANDLE);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY);
    }

    SECTION("External directory not a directory") {
        testPath /= "09-bad-external-dir";
        foeResultSet result = foeCreateYamlImporter(0, testPath.string().c_str(), &testImporter);

        CHECK(testImporter == FOE_NULL_HANDLE);
        CHECK(result.value == FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY);
    }
}