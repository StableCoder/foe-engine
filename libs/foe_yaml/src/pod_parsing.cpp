// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/yaml/exception.hpp>
#include <foe/yaml/export.h>
#include <foe/yaml/parsing.hpp>

#include <cstdint>

template <typename T>
bool yaml_read_optional(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        T &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        data = subNode.as<T>();
    } catch (...) {
        switch (node.Type()) {
        case YAML::NodeType::Null:
            throw foeYamlException(nodeName + " - Could not parse Null-type node as '" + typeName +
                                   "'");
        case YAML::NodeType::Scalar:
            throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                                   "' with value of: " + subNode.as<std::string>());
        case YAML::NodeType::Sequence:
            throw foeYamlException(nodeName + " - Could not parse Sequence-type node as '" +
                                   typeName + "'");
        case YAML::NodeType::Map:
            throw foeYamlException(nodeName + " - Could not parse Map-type node as '" + typeName +
                                   "'");
        case YAML::NodeType::Undefined:
            throw foeYamlException(nodeName + " - Could not parse Undefined-type node as '" +
                                   typeName + "'");
        }
    }

    return true;
}

template <typename T>
void yaml_read_required(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        T &data) {
    if (!yaml_read_optional(typeName, nodeName, node, data)) {
        throw foeYamlException(nodeName + " - Required node not found to parse as '" + typeName +
                               "'");
    }
}

template <typename T>
void yaml_write_required(std::string const &typeName,
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
        throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName + "'");
    }
}

template <typename T>
bool yaml_write_optional(std::string const &typeName,
                         std::string const &nodeName,
                         T const &defaultData,
                         T const &data,
                         YAML::Node &node) {
    if (data == defaultData) {
        return false;
    }

    yaml_write_required(typeName, nodeName, data, node);

    return true;
}

#define INSTANTIATION(T)                                                                           \
    template <>                                                                                    \
    FOE_YAML_EXPORT void yaml_read_required<T>(std::string const &nodeName,                        \
                                               YAML::Node const &node, T &data) {                  \
        yaml_read_required<T>(#T, nodeName, node, data);                                           \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_read_optional<T>(std::string const &nodeName,                        \
                                               YAML::Node const &node, T &data) {                  \
        return yaml_read_optional<T>(#T, nodeName, node, data);                                    \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT void yaml_write_required<T>(std::string const &nodeName, T const &data,        \
                                                YAML::Node &node) {                                \
        yaml_write_required<T>(#T, nodeName, data, node);                                          \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_write_optional<T>(std::string const &nodeName, T const &defaultData, \
                                                T const &data, YAML::Node &node) {                 \
        return yaml_write_optional<T>(#T, nodeName, defaultData, data, node);                      \
    }

INSTANTIATION(bool)

INSTANTIATION(int8_t)
INSTANTIATION(int16_t)
INSTANTIATION(int32_t)
INSTANTIATION(int64_t)

INSTANTIATION(uint8_t)
INSTANTIATION(uint16_t)
INSTANTIATION(uint32_t)
INSTANTIATION(uint64_t)

INSTANTIATION(float)
INSTANTIATION(double)

INSTANTIATION(std::string)