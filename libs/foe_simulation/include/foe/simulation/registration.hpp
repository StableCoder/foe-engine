// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_REGISTRATION_HPP
#define FOE_SIMULATION_REGISTRATION_HPP

#include <foe/error_code.h>
#include <foe/graphics/session.h>
#include <foe/simulation/export.h>

#include <stddef.h>

struct foeSimulation;
struct foeSimulationInitInfo;

typedef int foeSimulationUUID;

typedef foeResult (*PFN_foeSimulationCreate)(foeSimulation *);

/// @return Number of warnings/errors the occurred during the call.
typedef size_t (*PFN_foeSimulationDestroy)(foeSimulation *);

typedef foeResult (*PFN_foeSimulationInitialize)(foeSimulation *, foeSimulationInitInfo const *);

/// @return Number of warnings/errors the occurred during the call.
typedef size_t (*PFN_foeSimulationDeinitialize)(foeSimulation *);

typedef foeResult (*PFN_foeSimulationInitializeGraphics)(foeSimulation *, foeGfxSession);

/// @return Number of warnings/errors the occurred during the call.
typedef size_t (*PFN_foeSimulationDeinitializeGraphics)(foeSimulation *);

struct foeSimulationFunctionalty {
    /// The UUID of the functionality, must be valid/derived from the
    /// FOE_SIMULATION_FUNCTIONALITY_ID macro
    foeSimulationUUID id;
    /// Called on any created SimulationState, to create related data pools, uninitialized systems.
    PFN_foeSimulationCreate pCreateFn;
    /// Called when destroying any SimulationState to destroy related data pools and systems.
    PFN_foeSimulationDestroy pDestroyFn;

    /// To be called after pCreateFn when a Simulation is being initialized to start actually
    /// running a Simulation
    PFN_foeSimulationInitialize pInitializeFn;
    /// Called before pDestroyFn to safely destory any running state for an active SimulationState.
    /// Bool returns whether it completed cleanly.
    PFN_foeSimulationDeinitialize pDeinitializeFn;

    /// To be called when a graphics session is being added to a simulation
    PFN_foeSimulationInitializeGraphics pInitializeGraphicsFn;
    /// To be called when a graphics session is being removed from a simulation. Bool returns
    /// whether it completed cleanly.
    PFN_foeSimulationDeinitializeGraphics pDeinitializeGraphicsFn;
};

/**
 * @brief Attempts to register a set of simulation functionality globally
 * @param functionality Set of functionality to be registered
 * @return FOE_SIMULATION_SUCCESS on successful registration. A descriptive error code otherwise.
 *
 * First checks to see if the *exact* same set of functions already exists. If it does this returns
 * false. The same function may be used as part of a different set which would be counted as
 * 'different'.
 *
 * If there are any created simulations, then the provided 'pCreateFn' function is called on them.
 * If any simulations have been initialized prior, then the 'onInitialize' function is called on
 * them.
 *
 * If a failure occurs, then the functionality is fully removed from anywhere it may have succeeded
 * and is then not considered registered.
 */
FOE_SIM_EXPORT foeResult foeRegisterFunctionality(foeSimulationFunctionalty const &functionality);

/**
 * @brief Attempts to deregister a set of simulation functionality globally
 * @param functionalityUUID UUID of the functionality to deregister.
 * @return FOE_SIMULATION_SUCCESS on successful deregistration. A descriptive error code otherwise.
 *
 * If the functionality is found and to be deregistered, it will first iterate through all created
 * simulations and call 'pDeinitializeFn' and 'pDestroyFn' to remove the functionality before
 * finally returning.
 */
FOE_SIM_EXPORT foeResult foeDeregisterFunctionality(foeSimulationUUID functionalityUUID);

#endif // FOE_SIMULATION_REGISTRATION_HPP