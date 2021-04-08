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

#ifndef FOE_RESOURCE_YAML_MATERIAL_HPP
#define FOE_RESOURCE_YAML_MATERIAL_HPP

#include <foe/resource/material.hpp>
#include <foe/resource/yaml/export.h>
#include <yaml-cpp/yaml.h>

FOE_RES_YAML_EXPORT bool yaml_read_material_definition(YAML::Node const &node,
                                                       foeMaterialCreateInfo &createInfo);

FOE_RES_YAML_EXPORT void yaml_read_material_definition2(YAML::Node const &node,
                                                        foeResourceCreateInfoBase **ppCreateInfo);

FOE_RES_YAML_EXPORT bool export_yaml_material_definition(foeMaterial const *pMaterial);

#endif // FOE_RESOURCE_YAML_MATERIAL_HPP