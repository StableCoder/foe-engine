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

#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm_colour_parsing.hpp>
#include <foe/yaml/parsing.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <type_traits>

template <typename T>
bool yaml_read_optional(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        T &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    bool read = false;

    try {
        if (yaml_read_optional("x", subNode, data.x) || yaml_read_optional("r", subNode, data.x)) {
            read = true;
        }

        if constexpr ((std::is_same<T, glm::vec2>::value || std::is_same<T, glm::dvec2>::value ||
                       std::is_same<T, glm::bvec2>::value || std::is_same<T, glm::ivec2>::value ||
                       std::is_same<T, glm::uvec2>::value) ||
                      (std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (yaml_read_optional("y", subNode, data.y) ||
                yaml_read_optional("g", subNode, data.y)) {
                read = true;
            }
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (yaml_read_optional("z", subNode, data.z) ||
                yaml_read_optional("b", subNode, data.z)) {
                read = true;
            }
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (yaml_read_optional("w", subNode, data.w) ||
                yaml_read_optional("a", subNode, data.w)) {
                read = true;
            }
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                               "' with value of: " + subNode.as<std::string>());
    }

    return read;
}

template <typename T>
void yaml_read_required(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        T &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName + " - Required node not found to parse as '" + typeName +
                               "'");
    }

    try {
        if (!yaml_read_optional("x", subNode, data.x) &&
            !yaml_read_optional("r", subNode, data.x)) {
            throw foeYamlException(" - Neither 'x' nor 'r' nodes found to parse");
        }

        if constexpr ((std::is_same<T, glm::vec2>::value || std::is_same<T, glm::dvec2>::value ||
                       std::is_same<T, glm::bvec2>::value || std::is_same<T, glm::ivec2>::value ||
                       std::is_same<T, glm::uvec2>::value) ||
                      (std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (!yaml_read_optional("y", subNode, data.y) &&
                !yaml_read_optional("g", subNode, data.y)) {
                throw foeYamlException(" - Neither 'y' nor 'g' nodes found to parse");
            }
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (!yaml_read_optional("z", subNode, data.z) &&
                !yaml_read_optional("b", subNode, data.z)) {
                throw foeYamlException(" - Neither 'z' nor 'b' nodes found to parse");
            }
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (!yaml_read_optional("w", subNode, data.w) &&
                !yaml_read_optional("a", subNode, data.w)) {
                throw foeYamlException(" - Neither 'w' nor 'a' nodes found to parse");
            }
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.whatStr());
    }
}

template <typename T>
void yaml_write_required(std::string const &typeName,
                         std::string const &nodeName,
                         T const &data,
                         YAML::Node &node,
                         bool colour) {
    YAML::Node writeNode;
    YAML::Node *pNode = &writeNode;
    if (nodeName.empty()) {
        pNode = &node;
    }

    try {
        yaml_write_required((colour) ? "r" : "x", data.x, *pNode);

        if constexpr ((std::is_same<T, glm::vec2>::value || std::is_same<T, glm::dvec2>::value ||
                       std::is_same<T, glm::bvec2>::value || std::is_same<T, glm::ivec2>::value ||
                       std::is_same<T, glm::uvec2>::value) ||
                      (std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "g" : "y", data.y, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "b" : "z", data.z, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "a" : "w", data.w, *pNode);
        }

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName +
                               "' due to: " + e.whatStr());
    }

    if (!nodeName.empty()) {
        node[nodeName] = writeNode;
    }
}

template <typename T>
bool yaml_write_optional(std::string const &typeName,
                         std::string const &nodeName,
                         T const &defaultData,
                         T const &data,
                         YAML::Node &node,
                         bool colour) {
    if (data == defaultData) {
        return false;
    }

    YAML::Node writeNode;
    YAML::Node *pNode = &writeNode;
    if (nodeName.empty()) {
        pNode = &node;
    }

    try {
        yaml_write_required((colour) ? "r" : "x", data.x, *pNode);

        if constexpr ((std::is_same<T, glm::vec2>::value || std::is_same<T, glm::dvec2>::value ||
                       std::is_same<T, glm::bvec2>::value || std::is_same<T, glm::ivec2>::value ||
                       std::is_same<T, glm::uvec2>::value) ||
                      (std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "g" : "y", data.y, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "b" : "z", data.z, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write_required((colour) ? "a" : "w", data.w, *pNode);
        }
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName +
                               "' due to: " + e.whatStr());
    }

    if (!nodeName.empty()) {
        node[nodeName] = writeNode;
    }

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
        yaml_write_required<T>(#T, nodeName, data, node, false);                                   \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT void yaml_write_required_glm_colour<T>(std::string const &nodeName,            \
                                                           T const &data, YAML::Node &node) {      \
        yaml_write_required<T>(#T, nodeName, data, node, true);                                    \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_write_optional<T>(std::string const &nodeName, T const &defaultData, \
                                                T const &data, YAML::Node &node) {                 \
        return yaml_write_optional<T>(#T, nodeName, defaultData, data, node, false);               \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    FOE_YAML_EXPORT bool yaml_write_optional_glm_colour<T>(                                        \
        std::string const &nodeName, T const &defaultData, T const &data, YAML::Node &node) {      \
        return yaml_write_optional<T>(#T, nodeName, defaultData, data, node, true);                \
    }

// 4
INSTANTIATION(glm::vec4)
INSTANTIATION(glm::dvec4)
INSTANTIATION(glm::bvec4)
INSTANTIATION(glm::ivec4)
INSTANTIATION(glm::uvec4)
INSTANTIATION(glm::quat)

// 3
INSTANTIATION(glm::vec3)
INSTANTIATION(glm::dvec3)
INSTANTIATION(glm::bvec3)
INSTANTIATION(glm::ivec3)
INSTANTIATION(glm::uvec3)

// 2
INSTANTIATION(glm::vec2)
INSTANTIATION(glm::dvec2)
INSTANTIATION(glm::bvec2)
INSTANTIATION(glm::ivec2)
INSTANTIATION(glm::uvec2)

// 1
INSTANTIATION(glm::vec1)
INSTANTIATION(glm::dvec1)
INSTANTIATION(glm::bvec1)
INSTANTIATION(glm::ivec1)
INSTANTIATION(glm::uvec1)