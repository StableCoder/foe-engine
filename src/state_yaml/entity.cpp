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

#include <foe/ecs/groups.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../position_3d.hpp"

auto yaml_read_entity(YAML::Node const &node,
                      foeGroupID targetedGroupID,
                      std::vector<GroupTranslation> groupTranslations,
                      StatePools *pStatePools,
                      ResourcePools *pResourcePools) -> foeEntityID {
    { // *OPTIONAL* GroupID
        // If a GroupID is specified here, it means this data belongs to an entity from a
        // dependency, with some sort of data being overwritten
        uint64_t normalizedGroupID = 0;
        if (yaml_read_optional("group_id", node, normalizedGroupID)) {
            bool translationFound = false;
            for (auto const &it : groupTranslations) {
                if (it.source == normalizedGroupID) {
                    targetedGroupID = it.target;
                    translationFound = true;
                    break;
                }
            }
            if (!translationFound) {
                throw foeYamlException{"group_id - No group translation for value of '" +
                                       std::to_string(normalizedGroupID) + "' found"};
                return FOE_INVALID_ENTITY;
            }
        }
    }

    // *REQUIRED* IndexID
    uint64_t indexID = 0;
    yaml_read_required("index_id", node, indexID);

    // Determine full EntityID
    foeEntityID entity = targetedGroupID | indexID;

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

auto yaml_write_entity(foeEntityID entity, StatePools *pStatePools, ResourcePools *pResourcePools)
    -> YAML::Node {
    YAML::Node outNode;

    // *OPTIONAL* GroupID if *not* the persistent group
    if (foeEcsGetGroupID(entity) != foeEcsGroups::Persistent) {
        yaml_write_required("group_id", foeEcsGetNormalizedGroupID(entity), outNode);
    }

    // *REQUIRED* IndexID
    yaml_write_required("index_id", foeEcsGetIndexID(entity), outNode);

    // Position3D
    if (auto searchIt = pStatePools->position.find(entity);
        searchIt != pStatePools->position.end()) {
        outNode["position_3d"] = yaml_write_Position3D(*searchIt->second.get());
    }

    return outNode;
}