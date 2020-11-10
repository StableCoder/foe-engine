/*
    Copyright (C) 2020 George Cave.

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
        throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                               "' with value of: " + subNode.as<std::string>());
    }

    return true;
}

template <typename T>
bool yaml_read_required(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        T &data) {
    if (!yaml_read_optional(typeName, nodeName, node, data)) {
        throw foeYamlException(nodeName + " - Required node not found to parse as '" + typeName +
                               "'");
    }

    return true;
}

template <typename T>
bool yaml_write_required(std::string const &typeName,
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

    return true;
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

    return yaml_write_required(typeName, nodeName, data, node);
}

#define INSTANTIATION(T)                                                                           \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_read_required<T>(std::string const &nodeName,                        \
                                               YAML::Node const &node, T &data) {                  \
        return yaml_read_required<T>(#T, nodeName, node, data);                                    \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_read_optional<T>(std::string const &nodeName,                        \
                                               YAML::Node const &node, T &data) {                  \
        return yaml_read_optional<T>(#T, nodeName, node, data);                                    \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_write_required<T>(std::string const &nodeName, T const &data,        \
                                                YAML::Node &node) {                                \
        return yaml_write_required<T>(#T, nodeName, data, node);                                   \
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