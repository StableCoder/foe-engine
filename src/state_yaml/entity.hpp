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

#ifndef STATE_YAML_ENTITY_HPP
#define STATE_YAML_ENTITY_HPP

#include <foe/ecs/group_translation.hpp>
#include <foe/ecs/id.hpp>
#include <yaml-cpp/yaml.h>

#include "../resource_pools.hpp"
#include "../state_pools.hpp"

#include <vector>

auto yaml_read_entity(YAML::Node const &node,
                      foeIdGroup targetedGroupID,
                      foeGroupTranslation *pGroupTranslations,
                      StatePools *pStatePools,
                      ResourcePools *pResourcePools) -> foeId;

auto yaml_write_entity(foeId id, StatePools *pStatePools, ResourcePools *pResourcePools)
    -> YAML::Node;

#endif // STATE_YAML_ENTITY_HPP