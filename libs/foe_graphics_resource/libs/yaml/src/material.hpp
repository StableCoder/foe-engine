// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeMaterialCreateInfo;
struct foeGfxVkFragmentDescriptor;

char const *yaml_material_key();

void yaml_read_material(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo);

auto yaml_write_material(foeMaterialCreateInfo const &data,
                         foeGfxVkFragmentDescriptor *pFragmentDescriptor) -> YAML::Node;

#endif // MATERIAL_HPP