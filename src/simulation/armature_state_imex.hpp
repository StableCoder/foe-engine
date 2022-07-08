// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_IMEX_HPP
#define ARMATURE_STATE_IMEX_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "armature_state.hpp"

inline char const *yaml_armature_state_key() { return "armature_state"; }

inline auto yaml_read_ArmatureState(YAML::Node const &node, foeEcsGroupTranslator groupTranslator)
    -> foeArmatureState {
    foeArmatureState armatureState;

    yaml_read_id_optional("armature", node, groupTranslator, armatureState.armatureID);

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