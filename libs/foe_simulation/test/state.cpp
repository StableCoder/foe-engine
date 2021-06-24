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

#include <catch.hpp>
#include <foe/simulation/core.hpp>
#include <foe/simulation/error_code.hpp>
#include <foe/simulation/state.hpp>

namespace {
void onCreate(foeSimulationState *) {}
void onDestroy(foeSimulationState *) {}

void onCreate2(foeSimulationState *) {}
} // namespace

TEST_CASE("Core - De/Registering Functionality", "[foe][simulation]") {
    SECTION("Registering and deregistering the same set succeeds") {
        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_SUCCESS);

        REQUIRE(foeDeregisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_SUCCESS);
    }

    SECTION("Attempting to re-register the same set fails") {
        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_SUCCESS);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);

        REQUIRE(foeDeregisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_SUCCESS);
    }

    SECTION("Deregistering what wasn't registered fails") {
        REQUIRE(foeDeregisterFunctionality(foeSimulationFunctionalty{
                    .onCreate = onCreate,
                    .onDestroy = onDestroy,
                }) == FOE_SIMULATION_ERROR_FUNCTIONALITY_NOT_REGISTERED);
    }
}

TEST_CASE("SimState - EditorNameMap not created when addNameMaps set to false",
          "[foe][simulation]") {
    auto *pSimState = foeCreateSimulation({}, false);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap == nullptr);
    REQUIRE(pSimState->pEntityNameMap == nullptr);

    REQUIRE(foeDestroySimulation(pSimState) == FOE_SIMULATION_SUCCESS);
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true", "[foe][simulation]") {
    auto *pSimState = foeCreateSimulation({}, true);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap != nullptr);
    REQUIRE(pSimState->pEntityNameMap != nullptr);

    REQUIRE(foeDestroySimulation(pSimState) == FOE_SIMULATION_SUCCESS);
}