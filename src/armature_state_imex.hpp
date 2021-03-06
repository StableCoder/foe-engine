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

#ifndef ARMATURE_STATE_IMEX_HPP
#define ARMATURE_STATE_IMEX_HPP

#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "armature_state.hpp"

inline char const *yaml_armature_state_key() { return "armature_state"; }

inline auto yaml_read_ArmatureState(YAML::Node const &node, foeIdGroupTranslator const *pTranslator)
    -> foeArmatureState {
    foeArmatureState armatureState;

    yaml_read_id_optional("armature", node, pTranslator, armatureState.armatureID);

    yaml_read_required("animation", node, armatureState.animationID);
    yaml_read_required("time", node, armatureState.time);

    return armatureState;
}

inline auto yaml_write_ArmatureState(foeArmatureState const &data) -> YAML::Node {
    YAML::Node outNode;

    if (data.armatureID != FOE_INVALID_ID) {
        yaml_write_id("armature", data.armatureID, outNode);
    }

    yaml_write_required("animation", data.animationID, outNode);
    yaml_write_required("time", data.time, outNode);

    return outNode;
}

#endif // ARMATURE_STATE_IMEX_HPP