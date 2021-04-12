#ifndef SIMULATION_SET_HPP
#define SIMULATION_SET_HPP

#include <foe/ecs/editor_name_map.hpp>

#include "group_data.hpp"
#include "resource_pools.hpp"
#include "state_pools.hpp"

struct SimulationSet {
    foeGroupData groupData;
    foeEditorNameMap nameMap;
    // DataImporters importers;

    ResourcePools resources;
    ResourceLoaders resourceLoaders;

    StatePools state;
    // StateSystems systems;
};

#endif // SIMULATION_SET_HPP