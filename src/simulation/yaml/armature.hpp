/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef ARMATURE_HPP
#define ARMATURE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

struct foeArmatureCreateInfo;

char const *yaml_armature_key();

void yaml_read_armature(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo);

auto yaml_write_armature(foeArmatureCreateInfo const &data) -> YAML::Node;

#endif // ARMATURE_HPP