// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include "internal_pod_templates.hpp"

#include <cstdint>

template <typename T>
bool yaml_read(std::string const &typeName,
               std::string const &nodeName,
               YAML::Node const &node,
               T &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    try {
        data = readNode.as<T>();
    } catch (...) {
        switch (node.Type()) {
        case YAML::NodeType::Null:
            throw foeYamlException{nodeName + " - Could not parse Null-type node as '" + typeName +
                                   "'"};
        case YAML::NodeType::Scalar:
            throw foeYamlException{nodeName + " - Could not parse node as '" + typeName +
                                   "' with value of: " + readNode.as<std::string>()};
        case YAML::NodeType::Sequence:
            throw foeYamlException{nodeName + " - Could not parse Sequence-type node as '" +
                                   typeName + "'"};
        case YAML::NodeType::Map:
            throw foeYamlException{nodeName + " - Could not parse Map-type node as '" + typeName +
                                   "'"};
        case YAML::NodeType::Undefined:
            throw foeYamlException{nodeName + " - Could not parse Undefined-type node as '" +
                                   typeName + "'"};
        }
    }

    return true;
}

template <typename T>
void yaml_write(std::string const &typeName,
                std::string const &nodeName,
                T const &data,
                YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = data;
        } else {
            node[nodeName] = data;
        }
    } catch (...) {
        throw foeYamlException{nodeName + " - Failed to serialize node as '" + typeName + "'"};
    }
}

bool yaml_read_string(std::string const &nodeName, YAML::Node const &node, std::string &data) {
    return yaml_read<std::string>("string", nodeName, node, data);
}

void yaml_write_string(std::string const &nodeName, std::string const &data, YAML::Node &node) {
    yaml_write<std::string>("string", nodeName, data, node);
}

#define POD_YAML_FN_INSTANTIATION(T, Y)                                                            \
    bool yaml_read_##T(std::string const &nodeName, YAML::Node const &node, Y &data) {             \
        return yaml_read<Y>(#T, nodeName, node, data);                                             \
    }                                                                                              \
                                                                                                   \
    void yaml_write_##T(std::string const &nodeName, Y const &data, YAML::Node &node) {            \
        yaml_write<Y>(#T, nodeName, data, node);                                                   \
    }

#define POD_YAML_INSTANTIATION(T)                                                                  \
                                                                                                   \
    template <>                                                                                    \
    bool yaml_read<T>(std::string const &nodeName, YAML::Node const &node, T &data) {              \
        return yaml_read_##T(nodeName, node, data);                                                \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    void yaml_write<T>(std::string const &nodeName, T const &data, YAML::Node &node) {             \
        yaml_write_##T(nodeName, data, node);                                                      \
    }                                                                                              \
                                                                                                   \
    POD_YAML_FN_INSTANTIATION(T, T)

POD_YAML_INSTANTIATION(bool)

POD_YAML_FN_INSTANTIATION(int, int)
POD_YAML_INSTANTIATION(int8_t)
POD_YAML_INSTANTIATION(int16_t)
POD_YAML_INSTANTIATION(int32_t)
POD_YAML_INSTANTIATION(int64_t)

POD_YAML_FN_INSTANTIATION(unsigned_int, unsigned int)
POD_YAML_INSTANTIATION(uint8_t)
POD_YAML_INSTANTIATION(uint16_t)
POD_YAML_INSTANTIATION(uint32_t)
POD_YAML_INSTANTIATION(uint64_t)

POD_YAML_INSTANTIATION(float)
POD_YAML_INSTANTIATION(double)

#undef POD_YAML_INSTANTIATION