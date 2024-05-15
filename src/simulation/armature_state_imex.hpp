// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_STATE_IMEX_HPP
#define ARMATURE_STATE_IMEX_HPP

#include <foe/ecs/group_translator.h>
#include <yaml-cpp/yaml.h>

#include "armature_state.h"
#include "yaml/structs.hpp"

inline char const *yaml_armature_state_key() { return "armature_state"; }

inline auto yaml_read_ArmatureState(YAML::Node const &node,
                                    foeEcsGroupTranslator groupTranslator) -> foeArmatureState {
    foeArmatureState armatureState;

    yaml_read_foeArmatureState("", node, groupTranslator, armatureState);

    return armatureState;
}

inline auto yaml_write_ArmatureState(foeArmatureState const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_foeArmatureState("", data, outNode);

    return outNode;
}

#endif // ARMATURE_STATE_IMEX_HPP