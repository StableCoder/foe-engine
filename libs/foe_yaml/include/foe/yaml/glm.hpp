// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_YAML_GLM_HPP
#define FOE_YAML_GLM_HPP

#include <foe/yaml/export.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

#define GLM_YAML_DECLARATION(T)                                                                    \
    FOE_YAML_EXPORT                                                                                \
    bool yaml_read_glm_##T(std::string const &nodeName, YAML::Node const &node, glm::T &data);     \
                                                                                                   \
    FOE_YAML_EXPORT                                                                                \
    void yaml_write_glm_##T(std::string const &nodeName, glm::T const &data, YAML::Node &node);    \
                                                                                                   \
    FOE_YAML_EXPORT                                                                                \
    void yaml_write_glm_##T_colour(std::string const &nodeName, glm::T const &data,                \
                                   YAML::Node &node);

// 4
GLM_YAML_DECLARATION(vec4)
GLM_YAML_DECLARATION(dvec4)
GLM_YAML_DECLARATION(bvec4)
GLM_YAML_DECLARATION(ivec4)
GLM_YAML_DECLARATION(uvec4)
GLM_YAML_DECLARATION(quat)

// 3
GLM_YAML_DECLARATION(vec3)
GLM_YAML_DECLARATION(dvec3)
GLM_YAML_DECLARATION(bvec3)
GLM_YAML_DECLARATION(ivec3)
GLM_YAML_DECLARATION(uvec3)

// 2
GLM_YAML_DECLARATION(vec2)
GLM_YAML_DECLARATION(dvec2)
GLM_YAML_DECLARATION(bvec2)
GLM_YAML_DECLARATION(ivec2)
GLM_YAML_DECLARATION(uvec2)

// 1
GLM_YAML_DECLARATION(vec1)
GLM_YAML_DECLARATION(dvec1)
GLM_YAML_DECLARATION(bvec1)
GLM_YAML_DECLARATION(ivec1)
GLM_YAML_DECLARATION(uvec1)

#undef GLM_YAML_DECLARATION

#endif // FOE_YAML_GLM_HPP