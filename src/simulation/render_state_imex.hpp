// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_STATE_IMEX_HPP
#define RENDER_STATE_IMEX_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "render_state.hpp"

inline char const *yaml_render_state_key() { return "render_state"; }

inline auto yaml_read_RenderState(YAML::Node const &node, foeEcsGroupTranslator groupTranslator)
    -> foeRenderState {
    foeRenderState renderState;

    yaml_read_id_optional("vertex_descriptor", node, groupTranslator, renderState.vertexDescriptor);
    yaml_read_id_optional("boned_vertex_descriptor", node, groupTranslator,
                          renderState.bonedVertexDescriptor);
    yaml_read_id_optional("material", node, groupTranslator, renderState.material);
    yaml_read_id_optional("mesh", node, groupTranslator, renderState.mesh);

    return renderState;
}

inline auto yaml_write_RenderState(foeRenderState const &data) -> YAML::Node {
    YAML::Node outNode;

    if (data.vertexDescriptor != FOE_INVALID_ID) {
        yaml_write_id("vertex_descriptor", data.vertexDescriptor, outNode);
    }
    if (data.bonedVertexDescriptor != FOE_INVALID_ID) {
        yaml_write_id("boned_vertex_descriptor", data.bonedVertexDescriptor, outNode);
    }
    if (data.material != FOE_INVALID_ID) {
        yaml_write_id("material", data.material, outNode);
    }
    if (data.mesh != FOE_INVALID_ID) {
        yaml_write_id("mesh", data.mesh, outNode);
    }

    return outNode;
}

#endif // RENDER_STATE_IMEX_HPP