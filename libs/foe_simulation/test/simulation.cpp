/*
    Copyright (C) 2021-2022 George Cave.

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
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/simulation/type_defs.h>

namespace {
void onCreate(foeSimulationState *) {}
void onDestroy(foeSimulationState *) {}
} // namespace

constexpr int cTestFunctionalityID = FOE_SIMULATION_FUNCTIONALITY_ID(0);

TEST_CASE("Core - De/Registering Functionality", "[foe][simulation]") {
    SECTION("Registering and deregistering the same set succeeds") {
        REQUIRE_FALSE(foeRegisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));

        REQUIRE_FALSE(foeDeregisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));
    }

    SECTION("Attempting to re-register the same set fails") {
        REQUIRE_FALSE(foeRegisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);

        REQUIRE_FALSE(foeDeregisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));
    }

    SECTION("Registering different functionality with the same ID fails") {
        REQUIRE_FALSE(foeRegisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID,
                                             .onCreate = nullptr,
                                             .onDestroy = nullptr,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_ALREADY_IN_USE);

        REQUIRE_FALSE(foeDeregisterFunctionality(foeSimulationFunctionalty{
            .id = cTestFunctionalityID,
            .onCreate = onCreate,
            .onDestroy = onDestroy,
        }));
    }

    SECTION("Registering functionality with a invalid IDs fails") {
        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = 0,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = -1,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = -cTestFunctionalityID,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID + 1,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID - 1,
                                             .onCreate = onCreate,
                                             .onDestroy = onDestroy,
                                         })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_ID_INVALID);
    }

    SECTION("Deregistering what wasn't registered fails") {
        REQUIRE(foeDeregisterFunctionality(foeSimulationFunctionalty{
                                               .id = cTestFunctionalityID,
                                               .onCreate = onCreate,
                                               .onDestroy = onDestroy,
                                           })
                    .value() == FOE_SIMULATION_ERROR_FUNCTIONALITY_NOT_REGISTERED);
    }
}

TEST_CASE("SimState - EditorNameMap not created when addNameMaps set to false",
          "[foe][simulation]") {
    auto *pSimState = foeCreateSimulation(false);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap == nullptr);
    REQUIRE(pSimState->pEntityNameMap == nullptr);

    REQUIRE_FALSE(foeDestroySimulation(pSimState));
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true", "[foe][simulation]") {
    auto *pSimState = foeCreateSimulation(true);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap != nullptr);
    REQUIRE(pSimState->pEntityNameMap != nullptr);

    REQUIRE_FALSE(foeDestroySimulation(pSimState));
}