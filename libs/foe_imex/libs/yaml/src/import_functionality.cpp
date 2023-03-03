// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "import_functionality.hpp"
#include <foe/imex/yaml/importer.hpp>

#include "log.hpp"

#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace {

std::shared_mutex gSync;

std::map<std::string, foeImexYamlResourceFns> gResourceFns;
std::map<std::string, PFN_foeImexYamlComponent> gComponentFns;

} // namespace

auto sharedLockImportFunctionality() -> std::shared_lock<std::shared_mutex> {
    return std::shared_lock<std::shared_mutex>{gSync};
}

auto getResourceFns() -> std::map<std::string, foeImexYamlResourceFns> const & {
    return gResourceFns;
}

auto getComponentFns() -> std::map<std::string, PFN_foeImexYamlComponent> const & {
    return gComponentFns;
}

bool foeImexYamlRegisterResourceFns(std::string_view key, PFN_foeImexYamlResourceImport pImportFn) {
    auto localKey = std::string{key};
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(localKey);
    if (searchIt != gResourceFns.end()) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_ERROR,
                "Could not add Yaml Import function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_INFO, "Adding Yaml Import function for {}", key);
    gResourceFns[localKey] = foeImexYamlResourceFns{
        .pImport = pImportFn,
    };

    return true;
}

bool foeImexYamlDeregisterResourceFns(std::string_view key,
                                      PFN_foeImexYamlResourceImport pImportFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(std::string{key});
    if (searchIt == gResourceFns.end()) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_ERROR,
                "Could not remove Yaml Import function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second.pImport != pImportFn) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_WARNING,
                "Attempted to remove Yaml Import function for {}, but the provided "
                "function pointers are not the same as was added",
                key);
        return false;
    }

    gResourceFns.erase(searchIt);

    return true;
}

bool foeImexYamlRegisterComponentFn(std::string_view key, PFN_foeImexYamlComponent pImportFn) {
    auto localKey = std::string{key};
    std::unique_lock lock{gSync};

    auto searchIt = gComponentFns.find(localKey);
    if (searchIt != gComponentFns.end()) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_ERROR,
                "Could not add Yaml Import function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_INFO, "Adding Yaml Import function for {}", key);
    gComponentFns[localKey] = pImportFn;

    return true;
}

bool foeImexYamlDeregisterComponentFn(std::string_view key, PFN_foeImexYamlComponent pImportFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gComponentFns.find(std::string{key});
    if (searchIt == gComponentFns.end()) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_ERROR,
                "Could not remove Yaml Import function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second != pImportFn) {
        FOE_LOG(foeImexYaml, FOE_LOG_LEVEL_WARNING,
                "Attempted to remove Yaml Import function for {}, but the provided "
                "function pointers are not the same as was added",
                key);
        return false;
    }

    gComponentFns.erase(searchIt);

    return true;
}