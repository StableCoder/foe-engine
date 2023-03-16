// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <assimp/postprocess.h>
#include <catch2/catch_test_macros.hpp>
#include <foe/model/assimp/flags.hpp>

TEST_CASE("Serializing empty flags gives empty string", "[foe][model][assimp]") {}

TEST_CASE("Serializing aiProcess flags", "[foe][model][assimp]") {
    std::string serializedStr;

    SECTION("No flags results in empty string") {
        REQUIRE(foe_model_assimp_serialize(0, &serializedStr));
        REQUIRE(serializedStr.empty());
    }

    SECTION("First enum") {
        REQUIRE(foe_model_assimp_serialize(aiProcess_CalcTangentSpace, &serializedStr));
        REQUIRE(serializedStr == "CalcTangentSpace");
    }

    SECTION("Last enum") {
        REQUIRE(foe_model_assimp_serialize(aiProcess_Debone, &serializedStr));
        REQUIRE(serializedStr == "Debone");
    }

    SECTION("Cross-section of enums") {
        REQUIRE(foe_model_assimp_serialize(aiProcess_Triangulate | aiProcess_SortByPType |
                                               aiProcess_Debone | aiProcess_CalcTangentSpace,
                                           &serializedStr));
        REQUIRE(serializedStr == "CalcTangentSpace | Triangulate | SortByPType | Debone");
    }
}

TEST_CASE("Parsing aiProcess flags", "[foe][model][assimp]") {
    unsigned int parsed;

    SECTION("Empty string returns 0") {
        REQUIRE(foe_model_assimp_parse("", &parsed));
        REQUIRE(parsed == 0);
    }

    SECTION("First enum") {
        REQUIRE(foe_model_assimp_parse("CalcTangentSpace", &parsed));
        REQUIRE(parsed == aiProcess_CalcTangentSpace);

        SECTION("Mixed casing, spaces on sides") {
            REQUIRE(foe_model_assimp_parse("    \tcaLCtanGentSpacE       ", &parsed));
            REQUIRE(parsed == aiProcess_CalcTangentSpace);
        }
    }

    SECTION("Last enum") {
        REQUIRE(foe_model_assimp_parse("Debone", &parsed));
        REQUIRE(parsed == aiProcess_Debone);

        SECTION("Mixed casing, spaces on sides") {
            REQUIRE(foe_model_assimp_parse("  \n  DEBone\n \n \t", &parsed));
            REQUIRE(parsed == aiProcess_Debone);
        }
    }

    SECTION("Cross-section of enums") {
        REQUIRE(foe_model_assimp_parse("Debone | Triangulate | CalcTangentSpace | SortByPType",
                                       &parsed));
        REQUIRE(parsed == (aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_Debone |
                           aiProcess_CalcTangentSpace));

        SECTION("Mixed casing, spaces on sides") {
            REQUIRE(foe_model_assimp_parse(
                "\n\n\nDebone | \n\nTrianGULate \t| \tCalcTaNGentSpace | sortbyptype \t ",
                &parsed));
            REQUIRE(parsed == (aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_Debone |
                               aiProcess_CalcTangentSpace));
        }
    }

    SECTION("Non-existing enum fails") {
        REQUIRE_FALSE(foe_model_assimp_parse("DoesNotExistEnum", &parsed));
    }
    SECTION("Non-existing enum with existing fails") {
        REQUIRE_FALSE(
            foe_model_assimp_parse("Triangulate | DoesNotExistEnum | sortbyptype", &parsed));
    }
    SECTION("Non '|' character fails") {
        REQUIRE_FALSE(foe_model_assimp_parse("Triangulate & sortbyptype", &parsed));
    }
    SECTION("Non '|' character fails") {
        REQUIRE_FALSE(foe_model_assimp_parse("Triangulate 1231 sortbyptype", &parsed));
    }
}