// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/simulation/registration.h>
#include <foe/simulation/result.h>
#include <foe/simulation/simulation.h>
#include <foe/type_defs.h>

namespace {
foeResultSet pCreateFn(foeSimulation) { return {}; }
size_t pDestroyFn(foeSimulation) { return 0; }
} // namespace

constexpr foeSimulationUUID cTestFunctionalityID = FOE_PLUGIN_ID(0);

TEST_CASE("Core - De/Registering Functionality", "[foe][simulation]") {
    SECTION("Registering and deregistering the same set succeeds") {
        foeSimulationFunctionalty functionality = {
            .id = cTestFunctionalityID,
            .pCreateFn = pCreateFn,
            .pDestroyFn = pDestroyFn,
        };

        REQUIRE(foeRegisterFunctionality(&functionality).value == FOE_SUCCESS);

        REQUIRE(foeDeregisterFunctionality(cTestFunctionalityID).value == FOE_SUCCESS);
    }

    SECTION("Registering functionality with a invalid IDs fails") {
        foeSimulationFunctionalty functionality = {
            .id = -1,
            .pCreateFn = pCreateFn,
            .pDestroyFn = pDestroyFn,
        };

        REQUIRE(foeRegisterFunctionality(&functionality).value == FOE_SIMULATION_ERROR_ID_INVALID);

        functionality = {
            .id = -cTestFunctionalityID,
            .pCreateFn = pCreateFn,
            .pDestroyFn = pDestroyFn,
        };

        REQUIRE(foeRegisterFunctionality(&functionality).value == FOE_SIMULATION_ERROR_ID_INVALID);

        functionality = {
            .id = cTestFunctionalityID + 1,
            .pCreateFn = pCreateFn,
            .pDestroyFn = pDestroyFn,
        };

        REQUIRE(foeRegisterFunctionality(&functionality).value == FOE_SIMULATION_ERROR_ID_INVALID);

        functionality = {
            .id = cTestFunctionalityID - 1,
            .pCreateFn = pCreateFn,
            .pDestroyFn = pDestroyFn,
        };

        REQUIRE(foeRegisterFunctionality(&functionality).value == FOE_SIMULATION_ERROR_ID_INVALID);
    }

    SECTION("Deregistering what wasn't registered fails") {
        REQUIRE(foeDeregisterFunctionality(cTestFunctionalityID).value ==
                FOE_SIMULATION_ERROR_NOT_REGISTERED);
    }
}

TEST_CASE("SimState - EditorNameMap not created when addNameMaps set to false",
          "[foe][simulation]") {
    foeSimulation testSimulation{FOE_NULL_HANDLE};

    REQUIRE(foeCreateSimulation(false, &testSimulation).value == FOE_SUCCESS);

    REQUIRE(testSimulation != nullptr);
    REQUIRE(foeSimulationGetResourceNameMap(testSimulation) == FOE_NULL_HANDLE);
    REQUIRE(foeSimulationGetEntityNameMap(testSimulation) == FOE_NULL_HANDLE);

    REQUIRE(foeDestroySimulation(testSimulation).value == FOE_SUCCESS);
}

TEST_CASE("SimState - EditorNameMap created when addNameMaps set to true", "[foe][simulation]") {
    foeSimulation testSimulation{FOE_NULL_HANDLE};

    REQUIRE(foeCreateSimulation(true, &testSimulation).value == FOE_SUCCESS);

    REQUIRE(testSimulation != nullptr);
    REQUIRE(foeSimulationGetResourceNameMap(testSimulation) != FOE_NULL_HANDLE);
    REQUIRE(foeSimulationGetEntityNameMap(testSimulation) != FOE_NULL_HANDLE);

    REQUIRE(foeDestroySimulation(testSimulation).value == FOE_SUCCESS);
}
