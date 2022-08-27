// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_YAML_ID_HPP
#define FOE_ECS_YAML_ID_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/ecs/yaml/export.h>
#include <yaml-cpp/yaml.h>

FOE_ECS_YAML_EXPORT bool yaml_read_foeResourceID(std::string const &nodeName,
                                                 YAML::Node const &node,
                                                 foeEcsGroupTranslator groupTranslator,
                                                 foeResourceID &id);
FOE_ECS_YAML_EXPORT bool yaml_read_foeEntityID(std::string const &nodeName,
                                               YAML::Node const &node,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeEntityID &id);

FOE_ECS_YAML_EXPORT void yaml_write_foeResourceID(std::string const &nodeName,
                                                  foeResourceID id,
                                                  YAML::Node &node);
FOE_ECS_YAML_EXPORT void yaml_write_foeEntityID(std::string const &nodeName,
                                                foeEntityID id,
                                                YAML::Node &node);

#endif // FOE_ECS_YAML_ID_HPP