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

#ifndef FOE_SIMULATION_STATE_HPP
#define FOE_SIMULATION_STATE_HPP

#include <foe/simulation/core.hpp>
#include <foe/simulation/group_data.hpp>

#include <shared_mutex>
#include <vector>

class foeEditorNameMap;
struct foeResourceLoaderBase;
struct foeResourcePoolBase;
struct foeComponentPoolBase;
struct foeSystemBase;

struct foeSimulationState {
    /**
     * @brief Used to synchronize core access to the SimulationState
     *
     * Because this needs to support the possibility of registered core functionality operating
     * asynchronously, when it comes time to deinitialize or destroy functionality, we need to
     * *absolutely* sure that nothing is running asynchronously.
     *
     * To that end, the idea is that any asynchronous task will acquire the shared mutex's 'shared'
     * lock, and when the core needs to modify a SimulationState, then it acquires an exclusive lock
     * once all async tasks are complete, and then performs these modifications.
     *
     * Acquiring the exclusive lock should be an incredibly rare operation.
     */
    std::shared_mutex simSync;

    foeGroupData groupData;

    foeSimulationCreateInfo createInfo;
    // Information used to initialize functionality (used when functionality added during runtime)
    foeSimulationInitInfo initInfo{};

    foeEditorNameMap *pResourceNameMap;
    std::vector<foeResourceLoaderBase *> resourceLoaders;
    std::vector<foeResourcePoolBase *> resourcePools;

    foeEditorNameMap *pEntityNameMap;
    std::vector<foeComponentPoolBase *> componentPools;

    std::vector<foeSystemBase *> systems;
};

bool foeSimulationIsInitialized(foeSimulationState const *pSimulationState);

#endif // FOE_SIMULATION_STATE_HPP