// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeImageCreateInfo;

char const *yaml_image_key();

void yaml_read_image(YAML::Node const &node,
                     foeEcsGroupTranslator groupTranslator,
                     foeResourceCreateInfo *pCreateInfo);

auto yaml_write_image(foeImageCreateInfo const &data) -> YAML::Node;

#endif // IMAGE_HPP