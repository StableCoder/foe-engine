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

#ifndef FOE_SIMULATION_CORE_HPP
#define FOE_SIMULATION_CORE_HPP

#include <foe/simulation/export.h>

#include <foe/ecs/id.hpp>
#include <foe/graphics/session.hpp>

#include <filesystem>
#include <functional>
#include <mutex>
#include <vector>

/**
 * The Simulation 'core' is a static global registry of all simulation functionality that has been
 * added that is available.
 *
 * The idea is that, at runtime, libraries can be dynamically loaded and unloaded, and the
 * functionality specific to that library can be added/removed to all SimulationState sets, and be
 * used for dealing with the creation/initialization and deinitialization/destruction of these sets
 * safely.
 */

struct foeResourceCreateInfoBase;
struct foeResourcePoolBase;
struct foeResourceLoaderBase;
struct foeComponentPoolBase;
struct foeSystemBase;

struct foeSimulationState;

struct foeSimulationInitInfo {
    foeGfxSession gfxSession;
    std::function<foeResourceCreateInfoBase *(foeId)> resourceDefinitionImportFn;
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn;
    std::function<void(std::function<void()>)> asyncJobFn;

    foeResourcePoolBase **pResourcePools;
    uint32_t resourcePoolCount;

    foeResourceLoaderBase **pResourceLoaders;
    uint32_t resourceLoaderCount;

    foeComponentPoolBase **pComponentPools;
    uint32_t componentPoolCount;

    foeSystemBase **pSystems;
    uint32_t systemCount;
};

struct foeSimulationFunctionalty {
    void (*onCreate)(foeSimulationState *);
    void (*onDestroy)(foeSimulationState *);

    void (*onInitialization)(foeSimulationInitInfo const *);
    void (*onDeinitialization)(foeSimulationState const *);

    bool operator==(foeSimulationFunctionalty const &) const noexcept;
    bool operator!=(foeSimulationFunctionalty const &) const noexcept;
};

FOE_SIM_EXPORT bool foeRegisterFunctionality(foeSimulationFunctionalty const &functionality);
FOE_SIM_EXPORT void foeDeregisterFunctionality(foeSimulationFunctionalty const &functionality);

FOE_SIM_EXPORT foeSimulationState *foeCreateSimulation(bool addNameMaps);
FOE_SIM_EXPORT void foeDestroySimulation(foeSimulationState *pSimulationState);

FOE_SIM_EXPORT void foeInitializeSimulation(foeSimulationState *pSimulationState,
                                            foeSimulationInitInfo pInitInfo);
FOE_SIM_EXPORT void foeDeinitializeSimulation(foeSimulationState *pSimulationState);

#endif // FOE_SIMULATION_CORE_HPP