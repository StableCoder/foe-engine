// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_YAML_STRUCTS_HPP
#define FOE_PHYSICS_YAML_STRUCTS_HPP

#include <foe/ecs/group_translator.h>
#include <foe/physics/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

struct foeCollisionShapeCreateInfo;
struct foeRigidBody;

FOE_PHYSICS_YAML_EXPORT
bool yaml_read_foeCollisionShapeCreateInfo(std::string const &nodeName,
                                           YAML::Node const &node,
                                           foeCollisionShapeCreateInfo &data);

FOE_PHYSICS_YAML_EXPORT
void yaml_write_foeCollisionShapeCreateInfo(std::string const &nodeName,
                                            foeCollisionShapeCreateInfo const &data,
                                            YAML::Node &node);

FOE_PHYSICS_YAML_EXPORT
bool yaml_read_foeRigidBody(std::string const &nodeName,
                            YAML::Node const &node,
                            foeEcsGroupTranslator groupTranslator,
                            foeRigidBody &data);

FOE_PHYSICS_YAML_EXPORT
void yaml_write_foeRigidBody(std::string const &nodeName,
                             foeRigidBody const &data,
                             YAML::Node &node);

#endif // FOE_PHYSICS_YAML_STRUCTS_HPP
