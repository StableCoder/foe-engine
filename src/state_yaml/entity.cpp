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

#include <foe/ecs/editor_name_map.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/position/3d.hpp>
#include <foe/position/yaml/3d.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../armature_state.hpp"
#include "../render_state.hpp"
#include "../state_pools.hpp"

auto yaml_read_entity(YAML::Node const &node,
                      foeIdGroup targetedGroupID,
                      foeIdGroupTranslator *pGroupTranslator,
                      StatePools *pStatePools) -> foeId {
    foeId entity;
    yaml_read_id_required("", node, pGroupTranslator, entity);

    if (auto dataNode = node["position_3d"]; dataNode) {
        try {
            std::unique_ptr<Position3D> pPos(new Position3D);
            *pPos = yaml_read_Position3D(dataNode);
            pStatePools->position[entity] = std::move(pPos);
        } catch (foeYamlException const &e) {
            throw foeYamlException{"position_3d::" + e.whatStr()};
        }
    }

    if (auto dataNode = node["render_state"]; dataNode) {
        try {
            foeRenderState renderState = yaml_read_RenderState(dataNode, pGroupTranslator);
            pStatePools->renderStates[entity] = std::move(renderState);
        } catch (foeYamlException const &e) {
            throw foeYamlException{"render_state::" + e.whatStr()};
        }
    }

    if (auto dataNode = node["armature_state"]; dataNode) {
        try {
            foeArmatureState armatureState = yaml_read_ArmatureState(dataNode, pGroupTranslator);
            pStatePools->armatureStates[entity] = std::move(armatureState);
        } catch (foeYamlException const &e) {
            throw foeYamlException{"armature_state::" + e.whatStr()};
        }
    }

    return entity;
}

auto yaml_write_entity(foeId id, foeEditorNameMap *pNameMap, StatePools *pStatePools)
    -> YAML::Node {
    YAML::Node outNode;

    yaml_write_id("", id, outNode);

    if (pNameMap != nullptr) {
        auto name = pNameMap->find(id);

        if (!name.empty()) {
            yaml_write_required("editor_name", name, outNode);
        }
    }

    // Position3D
    if (auto searchIt = pStatePools->position.find(id); searchIt != pStatePools->position.end()) {
        outNode["position_3d"] = yaml_write_Position3D(*searchIt->second.get());
    }
    // RenderState
    if (auto searchIt = pStatePools->renderStates.find(id);
        searchIt != pStatePools->renderStates.end()) {
        outNode["render_state"] = yaml_write_RenderState(searchIt->second);
    }
    // ArmatureState
    if (auto searchIt = pStatePools->armatureStates.find(id);
        searchIt != pStatePools->armatureStates.end()) {
        outNode["armature_state"] = yaml_write_ArmatureState(searchIt->second);
    }

    return outNode;
}