// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/result.h>
#include <foe/simulation/simulation.hpp>
#include <foe/type_defs.h>

namespace {
foeResultSet pCreateFn(foeSimulation *) { return {}; }
size_t pDestroyFn(foeSimulation *) { return 0; }
} // namespace

constexpr foeSimulationUUID cTestFunctionalityID = FOE_PLUGIN_ID(0);

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
    REQUIRE(pSimState->resourceNameMap == FOE_NULL_HANDLE);
    REQUIRE(pSimState->entityNameMap == FOE_NULL_HANDLE);

    REQUIRE(foeDestroySimulation(pSimState).value == FOE_SUCCESS);
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true", "[foe][simulation]") {
    foeSimulation *pSimState{nullptr};

    REQUIRE(foeCreateSimulation(true, &pSimState).value == FOE_SUCCESS);

    REQUIRE(pSimState != nullptr);
    REQUIRE(pSimState->resourceNameMap != FOE_NULL_HANDLE);
    REQUIRE(pSimState->entityNameMap != FOE_NULL_HANDLE);

    REQUIRE(foeDestroySimulation(pSimState).value == FOE_SUCCESS);
}
