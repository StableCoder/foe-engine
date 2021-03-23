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

#ifndef FOE_RESOURCE_YAML_MESH_HPP
#define FOE_RESOURCE_YAML_MESH_HPP

#include <foe/resource/mesh_loader.hpp>
#include <foe/resource/yaml/export.h>

#include <string>
#include <string_view>

FOE_RES_YAML_EXPORT bool import_yaml_mesh_definition(std::string_view name,
                                                     std::unique_ptr<foeMeshCreateInfo> &meshCI);

#endif // FOE_RESOURCE_YAML_MESH_HPP