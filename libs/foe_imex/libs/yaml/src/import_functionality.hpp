// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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