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

#ifndef FOE_IMEX_YAML_EXPORTER_HPP
#define FOE_IMEX_YAML_EXPORTER_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <string>
#include <vector>

struct foeSimulation;

struct foeKeyYamlPair {
    std::string key;
    YAML::Node data;
};

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlExport(std::filesystem::path path,
                                                 foeSimulation *pSimState);

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlRegisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlDeregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlRegisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlDeregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *));

#endif // FOE_IMEX_YAML_EXPORTER_HPP