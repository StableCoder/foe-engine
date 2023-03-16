// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/ecs/group_translator.h>
#include <foe/ecs/result.h>

TEST_CASE("foeEcsGroupTranslator - Creation", "[foe][ecs]") {
    foeEcsGroupTranslator test{FOE_NULL_HANDLE};
    std::vector<char const *> sourceNames;
    std::vector<foeIdGroup> sourceIDs;
    std::vector<char const *> destinationNames;
    std::vector<foeIdGroup> destinationIDs;

    SECTION("Generating with no sources/destinations succeeds") {
        CHECK(foeEcsCreateGroupTranslator(sourceNames.size(), sourceNames.data(), sourceIDs.data(),
                                          destinationNames.size(), destinationNames.data(),
                                          destinationIDs.data(), &test)
                  .value == FOE_ECS_SUCCESS);
        CHECK(test != FOE_NULL_HANDLE);
    }

    SECTION("Generating with no sources but with destinations succeeds") {
        destinationNames = {"0", "1"};
        destinationIDs = {0, 1};

        CHECK(foeEcsCreateGroupTranslator(sourceNames.size(), sourceNames.data(), sourceIDs.data(),
                                          destinationNames.size(), destinationNames.data(),
                                          destinationIDs.data(), &test)
                  .value == FOE_ECS_SUCCESS);
        CHECK(test != FOE_NULL_HANDLE);
    }

    SECTION("Generating with sources and matching destinations succeeds") {
        sourceIDs = {1, 0};
        sourceNames = {"1", "0"};
        destinationIDs = {3, 4};
        destinationNames = {"0", "1"};

        CHECK(foeEcsCreateGroupTranslator(sourceNames.size(), sourceNames.data(), sourceIDs.data(),
                                          destinationNames.size(), destinationNames.data(),
                                          destinationIDs.data(), &test)
                  .value == FOE_ECS_SUCCESS);
        CHECK(test != FOE_NULL_HANDLE);
    }

    SECTION("Generating with sources with missing destinations fails") {
        sourceIDs = {1, 0};
        sourceNames = {"1", "0"};
        destinationNames = {"0"};
        destinationIDs = {0};

        CHECK(foeEcsCreateGroupTranslator(sourceNames.size(), sourceNames.data(), sourceIDs.data(),
                                          destinationNames.size(), destinationNames.data(),
                                          destinationIDs.data(), &test)
                  .value == FOE_ECS_ERROR_NO_MATCHING_GROUP);
        CHECK(test == FOE_NULL_HANDLE);
    }

    foeEcsDestroyGroupTranslator(test);
}

TEST_CASE("foeEcsGroupTranslator - Translating Values", "[foe][ecs]") {
    foeEcsGroupTranslator test{FOE_NULL_HANDLE};
    std::vector<char const *> sourceNames = {"1", "0"};
    std::vector<foeIdGroup> sourceIDs = {1, 0};
    std::vector<char const *> destinationNames = {"0", "1", "15"};
    std::vector<foeIdGroup> destinationIDs = {3, 4, 15};

    CHECK(foeEcsCreateGroupTranslator(sourceNames.size(), sourceNames.data(), sourceIDs.data(),
                                      destinationNames.size(), destinationNames.data(),
                                      destinationIDs.data(), &test)
              .value == FOE_ECS_SUCCESS);
    REQUIRE(test != FOE_NULL_HANDLE);

    SECTION("Original GroupID -> Translated GroupID") {
        SECTION("Valid original GroupIDs return valid translated GroupIDs") {
            foeIdGroup translatedGroup;

            CHECK(foeEcsGetTranslatedGroup(test, 0, &translatedGroup).value == FOE_ECS_SUCCESS);
            CHECK(translatedGroup == 3);

            CHECK(foeEcsGetTranslatedGroup(test, 1, &translatedGroup).value == FOE_ECS_SUCCESS);
            CHECK(translatedGroup == 4);
        }

        SECTION("Invalid original GroupIDs return FOE_ECS_ERROR_NO_MATCHING_GROUP and don't "
                "overwrite the output variable") {
            foeIdGroup translatedGroup = foeIdTemporaryGroup;

            CHECK(foeEcsGetTranslatedGroup(test, 14, &translatedGroup).value ==
                  FOE_ECS_ERROR_NO_MATCHING_GROUP);
            CHECK(translatedGroup == foeIdTemporaryGroup);

            CHECK(foeEcsGetTranslatedGroup(test, 15, &translatedGroup).value ==
                  FOE_ECS_ERROR_NO_MATCHING_GROUP);
            CHECK(translatedGroup == foeIdTemporaryGroup);
        }
    }

    SECTION("Translated GroupID -> Original GroupID") {
        SECTION("Valid translated GroupIDs return valid original GroupIDs") {
            foeIdGroup originalGroup;

            CHECK(foeEcsGetOriginalGroup(test, 3, &originalGroup).value == FOE_ECS_SUCCESS);
            CHECK(originalGroup == 0);

            CHECK(foeEcsGetOriginalGroup(test, 4, &originalGroup).value == FOE_ECS_SUCCESS);
            CHECK(originalGroup == 1);
        }

        SECTION("Invalid trnaslated GroupIDs return FOE_ECS_ERROR_NO_MATCHING_GROUP and don't "
                "overwrite the output variable") {
            foeIdGroup originalGroup = foeIdTemporaryGroup;

            CHECK(foeEcsGetOriginalGroup(test, 14, &originalGroup).value ==
                  FOE_ECS_ERROR_NO_MATCHING_GROUP);
            CHECK(originalGroup == foeIdTemporaryGroup);

            CHECK(foeEcsGetOriginalGroup(test, 15, &originalGroup).value ==
                  FOE_ECS_ERROR_NO_MATCHING_GROUP);
            CHECK(originalGroup == foeIdTemporaryGroup);
        }
    }

    foeEcsDestroyGroupTranslator(test);
}