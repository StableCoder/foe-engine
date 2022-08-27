#include <foe/model/assimp/flags.hpp>
#include <foe/yaml/pod.hpp>
#include <yaml-cpp/yaml.h>

inline bool yaml_read_aiPostProcessSteps(std::string const &nodeName,
                                         YAML::Node const &node,
                                         unsigned int &data) {
    std::string tmpStr;
    yaml_read_string(nodeName, node, tmpStr);

    return foe_model_assimp_parse(tmpStr, &data);
}

inline void yaml_write_aiPostProcessSteps(std::string const &nodeName,
                                          unsigned int const &data,
                                          YAML::Node &node) {
    std::string tmpStr;
    foe_model_assimp_serialize(data, &tmpStr);

    yaml_write_string(nodeName, tmpStr, node);
}