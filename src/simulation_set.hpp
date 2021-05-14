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

struct SimulationSet {
    foeGroupData groupData;

    ResourcePools resources;
    ResourceLoaders resourceLoaders;
    foeEditorNameMap resourceNameMap;

    StatePools state;
    foeEditorNameMap entityNameMap;
    // StateSystems systems;
};

#endif // SIMULATION_SET_HPP