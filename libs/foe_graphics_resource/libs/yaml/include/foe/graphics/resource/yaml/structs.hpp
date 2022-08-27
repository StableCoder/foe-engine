// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_YAML_STRUCTS_HPP
#define FOE_GRAPHICS_RESOURCE_YAML_STRUCTS_HPP

#include <foe/ecs/group_translator.h>
#include <foe/graphics/resource/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

struct foeImageCreateInfo;
struct foeMaterialCreateInfo;
struct foeMeshFileCreateInfo;
struct foeMeshIcosphereCreateInfo;
struct foeShaderCreateInfo;
struct foeVertexDescriptorCreateInfo;

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeImageCreateInfo(std::string const &nodeName,
                                                          YAML::Node const &node,
                                                          foeImageCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeImageCreateInfo(std::string const &nodeName,
                                                           foeImageCreateInfo const &data,
                                                           YAML::Node &node);

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeMaterialCreateInfo(std::string const &nodeName,
                                                             YAML::Node const &node,
                                                             foeEcsGroupTranslator groupTranslator,
                                                             foeMaterialCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeMaterialCreateInfo(std::string const &nodeName,
                                                              foeMaterialCreateInfo const &data,
                                                              YAML::Node &node);

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeMeshFileCreateInfo(std::string const &nodeName,
                                                             YAML::Node const &node,
                                                             foeMeshFileCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeMeshFileCreateInfo(std::string const &nodeName,
                                                              foeMeshFileCreateInfo const &data,
                                                              YAML::Node &node);

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeMeshIcosphereCreateInfo(std::string const &nodeName,
                                                                  YAML::Node const &node,
                                                                  foeMeshIcosphereCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeMeshIcosphereCreateInfo(
    std::string const &nodeName, foeMeshIcosphereCreateInfo const &data, YAML::Node &node);

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeShaderCreateInfo(std::string const &nodeName,
                                                           YAML::Node const &node,
                                                           foeShaderCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeShaderCreateInfo(std::string const &nodeName,
                                                            foeShaderCreateInfo const &data,
                                                            YAML::Node &node);

FOE_GFX_RES_YAML_EXPORT bool yaml_read_foeVertexDescriptorCreateInfo(
    std::string const &nodeName,
    YAML::Node const &node,
    foeEcsGroupTranslator groupTranslator,
    foeVertexDescriptorCreateInfo &data);

FOE_GFX_RES_YAML_EXPORT void yaml_write_foeVertexDescriptorCreateInfo(
    std::string const &nodeName, foeVertexDescriptorCreateInfo const &data, YAML::Node &node);

#endif // FOE_GRAPHICS_RESOURCE_YAML_STRUCTS_HPP
