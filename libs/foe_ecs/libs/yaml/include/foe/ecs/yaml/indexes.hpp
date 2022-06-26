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

#ifndef FOE_ECS_YAML_INDEXES_HPP
#define FOE_ECS_YAML_INDEXES_HPP

#include <foe/ecs/indexes.h>
#include <foe/ecs/yaml/export.h>
#include <yaml-cpp/yaml.h>

FOE_ECS_YAML_EXPORT void yaml_read_indexes(std::string const &nodeName,
                                           YAML::Node const &node,
                                           foeEcsIndexes indexes);

FOE_ECS_YAML_EXPORT void yaml_write_indexes(std::string const &nodeName,
                                            foeEcsIndexes indexes,
                                            YAML::Node &node);

#endif // FOE_ECS_YAML_INDEXES_HPP