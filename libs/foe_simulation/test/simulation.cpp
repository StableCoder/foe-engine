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
foeResult pCreateFn(foeSimulation *) { return {}; }
size_t pDestroyFn(foeSimulation *) { return 0; }
} // namespace

constexpr foeSimulationUUID cTestFunctionalityID = FOE_SIMULATION_FUNCTIONALITY_ID(0);

TEST_CASE("Core - De/Registering Functionality", "[foe][simulation]") {
    SECTION("Registering and deregistering the same set succeeds") {
        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SUCCESS);

        REQUIRE(foeDeregisterFunctionality(cTestFunctionalityID).value == FOE_SUCCESS);
    }

    SECTION("Registering functionality with a invalid IDs fails") {
        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = 0,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SIMULATION_ERROR_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = -1,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SIMULATION_ERROR_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = -cTestFunctionalityID,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SIMULATION_ERROR_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID + 1,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SIMULATION_ERROR_ID_INVALID);

        REQUIRE(foeRegisterFunctionality(foeSimulationFunctionalty{
                                             .id = cTestFunctionalityID - 1,
                                             .pCreateFn = pCreateFn,
                                             .pDestroyFn = pDestroyFn,
                                         })
                    .value == FOE_SIMULATION_ERROR_ID_INVALID);
    }

    SECTION("Deregistering what wasn't registered fails") {
        REQUIRE(foeDeregisterFunctionality(cTestFunctionalityID).value ==
                FOE_SIMULATION_ERROR_NOT_REGISTERED);
    }
}

TEST_CASE("SimState - EditorNameMap not created when addNameMaps set to false",
          "[foe][simulation]") {
    foeSimulation *pSimState{nullptr};

    REQUIRE(foeCreateSimulation(false, &pSimState).value == FOE_SUCCESS);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap == nullptr);
    REQUIRE(pSimState->pEntityNameMap == nullptr);

    REQUIRE(foeDestroySimulation(pSimState).value == FOE_SUCCESS);
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true", "[foe][simulation]") {
    foeSimulation *pSimState{nullptr};

    REQUIRE(foeCreateSimulation(true, &pSimState).value == FOE_SUCCESS);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->pResourceNameMap != nullptr);
    REQUIRE(pSimState->pEntityNameMap != nullptr);

    REQUIRE(foeDestroySimulation(pSimState).value == FOE_SUCCESS);
}
