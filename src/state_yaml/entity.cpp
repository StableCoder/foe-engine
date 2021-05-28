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
#include <foe/physics/yaml/component/rigid_body.hpp>
#include <foe/position/yaml/component/3d.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../armature_state.hpp"
#include "../camera.hpp"
#include "../render_state.hpp"
#include "../state_pools.hpp"

auto yaml_write_entity(foeId id, foeEditorNameMap *pEntityNameMap, StatePools *pStatePools)
    -> YAML::Node {
    YAML::Node outNode;

    yaml_write_id("", id, outNode);

    if (pEntityNameMap != nullptr) {
        auto name = pEntityNameMap->find(id);

        if (!name.empty()) {
            yaml_write_required("editor_name", name, outNode);
        }
    }

    // foePosition3d
    if (auto searchIt = pStatePools->position.find(id); searchIt != pStatePools->position.size()) {
        outNode["position_3d"] =
            yaml_write_Position3D(*pStatePools->position.begin<1>()[searchIt].get());
    }
    // Camera
    if (auto searchIt = pStatePools->camera.find(id); searchIt != pStatePools->camera.size()) {
        outNode["camera"] = yaml_write_Camera(*pStatePools->camera.begin<1>()[searchIt].get());
    }
    // RenderState
    if (auto searchIt = pStatePools->renderState.find(id);
        searchIt != pStatePools->renderState.size()) {
        outNode["render_state"] =
            yaml_write_RenderState(pStatePools->renderState.begin<1>()[searchIt]);
    }
    // ArmatureState
    if (auto searchIt = pStatePools->armatureState.find(id);
        searchIt != pStatePools->armatureState.size()) {
        outNode["armature_state"] =
            yaml_write_ArmatureState(pStatePools->armatureState.begin<1>()[searchIt]);
    }
    // RigidBody
    if (auto offset = pStatePools->rigidBody.find(id); offset != pStatePools->rigidBody.size()) {
        outNode["rigid_body"] = yaml_write_RigidBody(*(pStatePools->rigidBody.begin<1>() + offset));
    }

    return outNode;
}