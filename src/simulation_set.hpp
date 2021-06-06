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

#ifndef SIMULATION_SET_HPP
#define SIMULATION_SET_HPP

#include <foe/ecs/editor_name_map.hpp>
#include <foe/physics/system.hpp>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/state.hpp>

#include "resource_pools.hpp"
#include "state_pools.hpp"

#include <vector>

struct SimulationSet : public foeSimulationState {
    ResourcePools resources;
    ResourceLoaders resourceLoaders3;

    StatePools state;

    foePhysicsSystem physicsSystem;

    SimulationSet() {
        pResourceNameMap = new foeEditorNameMap;
        pEntityNameMap = new foeEditorNameMap;

        // Resource Loaders
        resourceLoaders.emplace_back(&resourceLoaders3.armature);
        resourceLoaders.emplace_back(&resourceLoaders3.collisionShape);

        resourceLoaders.emplace_back(&resourceLoaders3.shader);
        resourceLoaders.emplace_back(&resourceLoaders3.vertexDescriptor);
        resourceLoaders.emplace_back(&resourceLoaders3.image);
        resourceLoaders.emplace_back(&resourceLoaders3.material);
        resourceLoaders.emplace_back(&resourceLoaders3.mesh);

        // Resource Pools
        resourcePools.emplace_back(&resources.armature);
        resourcePools.emplace_back(&resources.collisionShape);

        resourcePools.emplace_back(&resources.shader);
        resourcePools.emplace_back(&resources.vertexDescriptor);
        resourcePools.emplace_back(&resources.image);
        resourcePools.emplace_back(&resources.material);
        resourcePools.emplace_back(&resources.mesh);

        // Component Pools
        componentPools.emplace_back(&state.armatureState);
        componentPools.emplace_back(&state.renderState);
        componentPools.emplace_back(&state.position);
        componentPools.emplace_back(&state.camera);
        componentPools.emplace_back(&state.rigidBody);

        // Systems
        systems.emplace_back(&physicsSystem);
    }

    ~SimulationSet() {
        delete pEntityNameMap;
        delete pResourceNameMap;
    }
};

#endif // SIMULATION_SET_HPP