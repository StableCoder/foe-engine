/*
    Copyright (C) 2022 George Cave.

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

#ifndef IMPORT_FUNCTIONALITY_HPP
#define IMPORT_FUNCTIONALITY_HPP

#include <foe/imex/yaml/importer.hpp>

#include <map>
#include <shared_mutex>
#include <tuple>

struct foeImexYamlResourceFns {
    PFN_foeImexYamlResourceImport pImport;
    PFN_foeImexYamlResourceCreate pCreate;
};

auto sharedLockImportFunctionality() -> std::shared_lock<std::shared_mutex>;

auto getResourceFns() -> std::map<std::string, foeImexYamlResourceFns> const &;

auto getComponentFns() -> std::map<std::string, PFN_foeImexYamlComponent> const &;

#endif // IMPORT_FUNCTIONALITY_HPP