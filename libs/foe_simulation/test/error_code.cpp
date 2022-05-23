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

#include <foe/simulation/error_code.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeSimulationResultToString(X, resultString);                                              \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeSimulationResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeSimulationResultToString((foeSimulationResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_SIMULATION_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeSimulationResultToString((foeSimulationResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_SIMULATION_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_NOT_REGISTERED)
    // Functionality Registration
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_ID_INVALID)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_ID_ALREADY_IN_USE)
    // Types
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_TYPE_NOT_FOUND)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_TYPE_ALREADY_EXISTS)
    // Simulation
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_SIMULATION_ALREADY_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_SIMULATION_NOT_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_ALREADY_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_SIMULATION_ERROR_SIMULATION_GRAPHICS_NOT_INITIALIZED)
}