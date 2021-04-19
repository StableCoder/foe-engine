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

#include "entity.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../position_3d.hpp"
#include "../state_pools.hpp"

auto yaml_read_entity(YAML::Node const &node,
                      foeIdGroup targetedGroupID,
                      foeIdGroupTranslator *pGroupTranslator,
                      StatePools *pStatePools) -> foeId {
    foeId entity;
    yaml_read_id_required("", node, pGroupTranslator, foeIdTypeEntity, entity);

    if (auto dataNode = node["position_3d"]; dataNode) {
        try {
            std::unique_ptr<Position3D> pPos(new Position3D);
            *pPos = yaml_read_Position3D(dataNode);
            pStatePools->position[entity] = std::move(pPos);
        } catch (foeYamlException const &e) {
            throw foeYamlException{"position_3d::" + e.whatStr()};
        }
    }

    return entity;
}

auto yaml_write_entity(foeId id, StatePools *pStatePools) -> YAML::Node {
    YAML::Node outNode;

    // yaml_write_id(id, outNode);

    // Position3D
    if (auto searchIt = pStatePools->position.find(id); searchIt != pStatePools->position.end()) {
        outNode["position_3d"] = yaml_write_Position3D(*searchIt->second.get());
    }

    return outNode;
}