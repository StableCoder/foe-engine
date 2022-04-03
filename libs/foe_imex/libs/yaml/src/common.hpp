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

#ifndef COMMON_HPP
#define COMMON_HPP

#include <string_view>

constexpr std::string_view dependenciesFilePath = "dependencies.yml";

constexpr std::string_view resourceIndexDataFilePath = "resource_index_data.yml";
constexpr std::string_view resourceDirectoryPath = "resources";

constexpr std::string_view entityIndexDataFilePath = "entity_index_data.yml";
constexpr std::string_view entityDirectoryPath = "entities";

constexpr std::string_view externalDirectoryPath = "external";

#endif // COMMON_HPP