#ifndef RENDER_STATE_HPP
#define RENDER_STATE_HPP

#include <foe/ecs/id.hpp>

struct foeRenderState {
    foeId vertexDescriptor;
    foeId bonedVertexDescriptor;
    foeId material;
    foeId mesh;
};

#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

inline auto yaml_read_RenderState(YAML::Node const &node, foeIdGroupTranslator const *pTranslator)
    -> foeRenderState {
    foeRenderState renderState;

    yaml_read_id_optional("vertex_descriptor", node, pTranslator, foeIdTypeResource,
                          renderState.vertexDescriptor);
    yaml_read_id_optional("boned_vertex_descriptor", node, pTranslator, foeIdTypeResource,
                          renderState.bonedVertexDescriptor);
    yaml_read_id_optional("material", node, pTranslator, foeIdTypeResource, renderState.material);
    yaml_read_id_optional("mesh", node, pTranslator, foeIdTypeResource, renderState.mesh);

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

#endif // RENDER_STATE_HPP