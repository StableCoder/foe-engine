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

#ifndef RENDER_STATE_IMEX_HPP
#define RENDER_STATE_IMEX_HPP

#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "render_state.hpp"

inline char const *yaml_render_state_key() { return "render_state"; }

inline auto yaml_read_RenderState(YAML::Node const &node, foeIdGroupTranslator const *pTranslator)
    -> foeRenderState {
    foeRenderState renderState;

    yaml_read_id_optional("vertex_descriptor", node, pTranslator, renderState.vertexDescriptor);
    yaml_read_id_optional("boned_vertex_descriptor", node, pTranslator,
                          renderState.bonedVertexDescriptor);
    yaml_read_id_optional("material", node, pTranslator, renderState.material);
    yaml_read_id_optional("mesh", node, pTranslator, renderState.mesh);

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