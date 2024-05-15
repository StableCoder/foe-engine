// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_STATE_IMEX_HPP
#define RENDER_STATE_IMEX_HPP

#include <foe/ecs/group_translator.h>
#include <yaml-cpp/yaml.h>

#include "render_state.h"
#include "yaml/structs.hpp"

inline char const *yaml_render_state_key() { return "render_state"; }

inline auto yaml_read_RenderState(YAML::Node const &node,
                                  foeEcsGroupTranslator groupTranslator) -> foeRenderState {
    foeRenderState renderState;

    yaml_read_foeRenderState("", node, groupTranslator, renderState);

    return renderState;
}

inline auto yaml_write_RenderState(foeRenderState const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_foeRenderState("", data, outNode);

    return outNode;
}

#endif // RENDER_STATE_IMEX_HPP