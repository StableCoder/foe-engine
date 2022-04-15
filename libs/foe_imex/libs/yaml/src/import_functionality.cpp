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

#include "import_functionality.hpp"
#include <foe/imex/yaml/importer.hpp>

#include "log.hpp"

#include <map>
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

bool foeImexYamlRegisterResourceFns(std::string_view key,
                                    PFN_foeImexYamlResourceImport pImportFn,
                                    PFN_foeImexYamlResourceCreate pCreateFn) {
    auto localKey = std::string{key};
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(localKey);
    if (searchIt != gResourceFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not add Yaml Import function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, Info, "Adding Yaml Import function for {}", key);
    gResourceFns[localKey] = foeImexYamlResourceFns{
        .pImport = pImportFn,
        .pCreate = pCreateFn,
    };

    return true;
}

bool foeImexYamlDeregisterResourceFns(std::string_view key,
                                      PFN_foeImexYamlResourceImport pImportFn,
                                      PFN_foeImexYamlResourceCreate pCreateFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(std::string{key});
    if (searchIt == gResourceFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not remove Yaml Import function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second.pImport != pImportFn || searchIt->second.pCreate != pCreateFn) {
        FOE_LOG(foeImexYaml, Warning,
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
        FOE_LOG(foeImexYaml, Error,
                "Could not add Yaml Import function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, Info, "Adding Yaml Import function for {}", key);
    gComponentFns[localKey] = pImportFn;

    return true;
}

bool foeImexYamlDeregisterComponentFn(std::string_view key, PFN_foeImexYamlComponent pImportFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gComponentFns.find(std::string{key});
    if (searchIt == gComponentFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not remove Yaml Import function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second != pImportFn) {
        FOE_LOG(foeImexYaml, Warning,
                "Attempted to remove Yaml Import function for {}, but the provided "
                "function pointers are not the same as was added",
                key);
        return false;
    }

    gComponentFns.erase(searchIt);

    return true;
}