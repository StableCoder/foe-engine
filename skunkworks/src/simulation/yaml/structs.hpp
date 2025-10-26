// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <foe/ecs/group_translator.h>
#include <yaml-cpp/yaml.h>

#include <string>

struct AnimationImportInfo;
struct foeArmatureCreateInfo;
struct foeArmatureState;
struct foeCamera;
struct foeRenderState;

bool yaml_read_AnimationImportInfo(std::string const &nodeName,
                                   YAML::Node const &node,
                                   AnimationImportInfo &data);

void yaml_write_AnimationImportInfo(std::string const &nodeName,
                                    AnimationImportInfo const &data,
                                    YAML::Node &node);
bool yaml_read_foeArmatureCreateInfo(std::string const &nodeName,
                                     YAML::Node const &node,
                                     foeArmatureCreateInfo &data);

void yaml_write_foeArmatureCreateInfo(std::string const &nodeName,
                                      foeArmatureCreateInfo const &data,
                                      YAML::Node &node);
bool yaml_read_foeArmatureState(std::string const &nodeName,
                                YAML::Node const &node,
                                foeEcsGroupTranslator groupTranslator,
                                foeArmatureState &data);

void yaml_write_foeArmatureState(std::string const &nodeName,
                                 foeArmatureState const &data,
                                 YAML::Node &node);

bool yaml_read_foeRenderState(std::string const &nodeName,
                              YAML::Node const &node,
                              foeEcsGroupTranslator groupTranslator,
                              foeRenderState &data);

void yaml_write_foeRenderState(std::string const &nodeName,
                               foeRenderState const &data,
                               YAML::Node &node);

#endif // STRUCTS_HPP
