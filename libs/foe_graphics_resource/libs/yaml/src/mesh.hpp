// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_HPP
#define MESH_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeMeshFileCreateInfo;
struct foeMeshCubeCreateInfo;
struct foeMeshIcosphereCreateInfo;

char const *yaml_mesh_file_key();

void yaml_read_mesh_file(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeResourceCreateInfo *pCreateInfo);

auto yaml_write_mesh_file(foeMeshFileCreateInfo const &data) -> YAML::Node;

char const *yaml_mesh_cube_key();

void yaml_read_mesh_cube(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeResourceCreateInfo *pCreateInfo);

auto yaml_write_mesh_cube(foeMeshCubeCreateInfo const &data) -> YAML::Node;

char const *yaml_mesh_icosphere_key();

void yaml_read_mesh_icosphere(YAML::Node const &node,
                              foeEcsGroupTranslator groupTranslator,
                              foeResourceCreateInfo *pCreateInfo);

auto yaml_write_mesh_icosphere(foeMeshIcosphereCreateInfo const &data) -> YAML::Node;

#endif // MESH_HPP