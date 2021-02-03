#include <foe/resource/yaml/shader.hpp>

#include <foe/graphics/yaml/shader.hpp>
#include <foe/yaml/exception.hpp>

bool yaml_write_shader_declaration(std::string const &nodeName,
                                   foeShader const *pShader,
                                   YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = std::string{pShader->getName()};
        } else {
            node[nodeName] = std::string{pShader->getName()};
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeShader' declaration");
    }

    return true;
}

bool yaml_write_shader_definition(std::string const &nodeName,
                                  foeGfxSession session,
                                  foeShader const *pShader,
                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Resources Node
        // (Nothing Currently)

        // Gfx Data Node
        yaml_write_gfx_shader("graphics_data", session, pShader->getShader(), writeNode);
    } catch (...) {
        throw foeYamlException(nodeName +
                               " - Failed to serialize 'foeFragmentDescriptor' definition");
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}