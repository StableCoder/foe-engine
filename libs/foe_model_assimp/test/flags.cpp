/*
    Copyright (C) 2021 George Cave.

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

#include <assimp/postprocess.h>
#include <catch.hpp>
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
        REQUIRE(foe_model_assimp_serialize(aiProcess_GenBoundingBoxes, &serializedStr));
        REQUIRE(serializedStr == "GenBoundingBoxes");
    }

    SECTION("Cross-section of enums") {
        REQUIRE(foe_model_assimp_serialize(aiProcess_Triangulate | aiProcess_SortByPType |
                                               aiProcess_GenBoundingBoxes |
                                               aiProcess_CalcTangentSpace,
                                           &serializedStr));
        REQUIRE(serializedStr == "CalcTangentSpace | Triangulate | SortByPType | GenBoundingBoxes");
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
        REQUIRE(foe_model_assimp_parse("GenBoundingBoxes", &parsed));
        REQUIRE(parsed == aiProcess_GenBoundingBoxes);

        SECTION("Mixed casing, spaces on sides") {
            REQUIRE(foe_model_assimp_parse("  \n  GENBOUNDingboxes\n \n \t", &parsed));
            REQUIRE(parsed == aiProcess_GenBoundingBoxes);
        }
    }

    SECTION("Cross-section of enums") {
        REQUIRE(foe_model_assimp_parse(
            "GenBoundingBoxes | Triangulate | CalcTangentSpace | SortByPType", &parsed));
        REQUIRE(parsed == (aiProcess_Triangulate | aiProcess_SortByPType |
                           aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace));

        SECTION("Mixed casing, spaces on sides") {
            REQUIRE(foe_model_assimp_parse(
                "\n\n\nGenBoundingBoxes | \n\nTrianGULate \t| \tCalcTaNGentSpace | sortbyptype \t ",
                &parsed));
            REQUIRE(parsed == (aiProcess_Triangulate | aiProcess_SortByPType |
                               aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace));
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