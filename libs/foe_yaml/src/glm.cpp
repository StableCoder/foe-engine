// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "internal_pod_templates.hpp"

#include <type_traits>

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
        if (!yaml_read("x", readNode, data.x) && !yaml_read("r", readNode, data.x)) {
            return false;
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
            if (!yaml_read("y", readNode, data.y) && !yaml_read("g", readNode, data.y)) {
                return false;
            }
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (!yaml_read("z", readNode, data.z) && !yaml_read("b", readNode, data.z)) {
                return false;
            }
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            if (!yaml_read("w", readNode, data.w) && !yaml_read("a", readNode, data.w)) {
                return false;
            }
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Could not parse node as '" + typeName +
                               "' with value of: " + readNode.as<std::string>());
    }

    return true;
}

template <typename T>
void yaml_write(std::string const &typeName,
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
        yaml_write((colour) ? "r" : "x", data.x, *pNode);

        if constexpr ((std::is_same<T, glm::vec2>::value || std::is_same<T, glm::dvec2>::value ||
                       std::is_same<T, glm::bvec2>::value || std::is_same<T, glm::ivec2>::value ||
                       std::is_same<T, glm::uvec2>::value) ||
                      (std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write((colour) ? "g" : "y", data.y, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec3>::value || std::is_same<T, glm::dvec3>::value ||
                       std::is_same<T, glm::bvec3>::value || std::is_same<T, glm::ivec3>::value ||
                       std::is_same<T, glm::uvec3>::value) ||
                      (std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write((colour) ? "b" : "z", data.z, *pNode);
        }

        if constexpr ((std::is_same<T, glm::vec4>::value || std::is_same<T, glm::dvec4>::value ||
                       std::is_same<T, glm::bvec4>::value || std::is_same<T, glm::ivec4>::value ||
                       std::is_same<T, glm::uvec4>::value || std::is_same<T, glm::quat>::value)) {
            yaml_write((colour) ? "a" : "w", data.w, *pNode);
        }

    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + " - Failed to serialize node as '" + typeName +
                               "' due to: " + e.whatStr());
    }

    if (!nodeName.empty()) {
        node[nodeName] = writeNode;
    }
}

#define INSTANTIATION(T)                                                                           \
    bool yaml_read_glm_##T(std::string const &nodeName, YAML::Node const &node, glm::T &data) {    \
        return yaml_read<glm::T>(#T, nodeName, node, data);                                        \
    }                                                                                              \
                                                                                                   \
    void yaml_write_glm_##T(std::string const &nodeName, glm::T const &data, YAML::Node &node) {   \
        yaml_write<glm::T>(#T, nodeName, data, node, false);                                       \
    }                                                                                              \
                                                                                                   \
    void yaml_write_glm_##T_colour(std::string const &nodeName, glm::T const &data,                \
                                   YAML::Node &node) {                                             \
        yaml_write<glm::T>(#T, nodeName, data, node, true);                                        \
    }

// 4
INSTANTIATION(vec4)
INSTANTIATION(dvec4)
INSTANTIATION(bvec4)
INSTANTIATION(ivec4)
INSTANTIATION(uvec4)
INSTANTIATION(quat)

// 3
INSTANTIATION(vec3)
INSTANTIATION(dvec3)
INSTANTIATION(bvec3)
INSTANTIATION(ivec3)
INSTANTIATION(uvec3)

// 2
INSTANTIATION(vec2)
INSTANTIATION(dvec2)
INSTANTIATION(bvec2)
INSTANTIATION(ivec2)
INSTANTIATION(uvec2)

// 1
INSTANTIATION(vec1)
INSTANTIATION(dvec1)
INSTANTIATION(bvec1)
INSTANTIATION(ivec1)
INSTANTIATION(uvec1)