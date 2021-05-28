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

#include "group_data.hpp"
#include "resource_pools.hpp"
#include "state_pools.hpp"

#include <vector>

struct foeResourceLoaderBase;
struct foeResourcePoolBase;

struct SimulationSet {
    foeGroupData groupData;

    std::vector<foeResourceLoaderBase *> resourceLoaders2;
    std::vector<foeResourcePoolBase *> resourcePools;
    ResourcePools resources;
    ResourceLoaders resourceLoaders;
    foeEditorNameMap resourceNameMap;

    std::vector<foeComponentPoolBase *> componentPools;
    StatePools state;
    foeEditorNameMap entityNameMap;

    SimulationSet() {
        // Resource Loaders
        resourceLoaders2.emplace_back(&resourceLoaders.armature);
        resourceLoaders2.emplace_back(&resourceLoaders.collisionShape);

        resourceLoaders2.emplace_back(&resourceLoaders.shader);
        resourceLoaders2.emplace_back(&resourceLoaders.vertexDescriptor);
        resourceLoaders2.emplace_back(&resourceLoaders.image);
        resourceLoaders2.emplace_back(&resourceLoaders.material);
        resourceLoaders2.emplace_back(&resourceLoaders.mesh);

        // Resource Pools
        resourcePools.emplace_back(&resources.armature);
        resourcePools.emplace_back(&resources.collisionShape);

        resourcePools.emplace_back(&resources.shader);
        resourcePools.emplace_back(&resources.vertexDescriptor);
        resourcePools.emplace_back(&resources.image);
        resourcePools.emplace_back(&resources.material);
        resourcePools.emplace_back(&resources.mesh);

        // Component Pools
        componentPools.emplace_back(&state.position);
        componentPools.emplace_back(&state.camera);
        componentPools.emplace_back(&state.rigidBody);
    }
};

#endif // SIMULATION_SET_HPP