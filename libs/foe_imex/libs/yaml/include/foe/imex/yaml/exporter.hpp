// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlRegisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlDeregisterResourceFn(
    std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlRegisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *));

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlDeregisterComponentFn(
    std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID, foeSimulation const *));

#endif // FOE_IMEX_YAML_EXPORTER_HPP