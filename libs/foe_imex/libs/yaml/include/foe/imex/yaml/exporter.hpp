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

#ifndef FOE_IMEX_YAML_EXPORTER_HPP
#define FOE_IMEX_YAML_EXPORTER_HPP

#include <foe/ecs/id.hpp>
#include <foe/imex/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

struct foeSimulationState;
struct foeResourcePoolBase;
struct foeComponentPoolBase;

struct foeKeyYamlPair {
    std::string key;
    YAML::Node data;
};

FOE_IMEX_YAML_EXPORT std::error_code foeImexYamlExport(std::filesystem::path path,
                                                       foeSimulationState *pSimState);

FOE_IMEX_YAML_EXPORT std::error_code foeImexYamlRegisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeResourcePoolBase **, uint32_t));

FOE_IMEX_YAML_EXPORT std::error_code foeImexYamlDeregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeResourcePoolBase **, uint32_t));

FOE_IMEX_YAML_EXPORT std::error_code foeImexYamlRegisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeComponentPoolBase **, uint32_t));

FOE_IMEX_YAML_EXPORT std::error_code foeImexYamlDeregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeComponentPoolBase **, uint32_t));

#endif // FOE_IMEX_YAML_EXPORTER_HPP