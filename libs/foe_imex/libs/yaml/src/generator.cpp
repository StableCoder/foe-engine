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

#include <foe/imex/yaml/generator.hpp>

#include <foe/imex/yaml/importer.hpp>

#include "log.hpp"

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view resourceIndexDataFilePath = "resource_index_data.yml";
constexpr std::string_view resourceDirectoryPath = "resources";
constexpr std::string_view externalDirectoryPath = "external";
constexpr std::string_view stateIndexDataFilePath = "state_index_data.yml";
constexpr std::string_view stateDirectoryPath = "state";

} // namespace

auto foeYamlImporterGenerator::createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
    -> foeImporterBase * {
    if (std::filesystem::is_directory(stateDataPath) &&
        // Dependencies
        (std::filesystem::exists(stateDataPath / dependenciesFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / dependenciesFilePath)) &&
        // Resource Index Data
        (std::filesystem::exists(stateDataPath / resourceIndexDataFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / resourceIndexDataFilePath)) &&
        // State Index Data
        (std::filesystem::exists(stateDataPath / stateIndexDataFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / stateIndexDataFilePath))) {

        // Check optional directories (state data, resources and external data), fail if they fail

        // Resources Directory
        if (std::filesystem::exists(stateDataPath / resourceDirectoryPath) &&
            !std::filesystem::is_directory(stateDataPath / resourceDirectoryPath))
            return nullptr;

        // External Data Directory
        if (std::filesystem::exists(stateDataPath / externalDirectoryPath) &&
            !std::filesystem::is_directory(stateDataPath / externalDirectoryPath))
            return nullptr;

        // State Data Directory
        if (std::filesystem::exists(stateDataPath / stateDirectoryPath) &&
            !std::filesystem::is_directory(stateDataPath / stateDirectoryPath))
            return nullptr;

        return new foeYamlImporter{this, group, stateDataPath};
    }

    return nullptr;
}

bool foeYamlImporterGenerator::addImporter(std::string key,
                                           ImportFn pImportFn,
                                           CreateFn pCreateFn) {
    auto searchIt = mResourceFns.find(key);
    if (searchIt != mResourceFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not add DistributedYamlImporter function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, Info, "Adding DistributedYamlImporter function for {}", key);
    mResourceFns[key] = ResourceFunctions{
        .pImport = pImportFn,
        .pCreate = pCreateFn,
    };
    return true;
}

bool foeYamlImporterGenerator::removeImporter(std::string key,
                                              ImportFn pImportFn,
                                              CreateFn pCreateFn) {
    auto searchIt = mResourceFns.find(key);
    if (searchIt == mResourceFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not remove DistributedYamlImporter function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second.pImport != pImportFn || searchIt->second.pCreate != pCreateFn) {
        FOE_LOG(foeImexYaml, Warning,
                "Attempted to remove DistributedYamlImporter function for {}, but the provided "
                "function pointers are not the same as was added",
                key);
        return false;
    }

    mResourceFns.erase(searchIt);
    return true;
}

bool foeYamlImporterGenerator::addComponentImporter(std::string key, ComponentImportFn pImportFn) {
    auto searchIt = mComponentFns.find(key);
    if (searchIt != mComponentFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not add DistributedYamlImporter function for {}, as it already exists", key);
        return false;
    }

    FOE_LOG(foeImexYaml, Info, "Adding DistributedYamlImporter function for {}", key);
    mComponentFns[key] = pImportFn;

    return true;
}

bool foeYamlImporterGenerator::removeComponentImporter(std::string key,
                                                       ComponentImportFn pImportFn) {
    auto searchIt = mComponentFns.find(key);
    if (searchIt == mComponentFns.end()) {
        FOE_LOG(foeImexYaml, Error,
                "Could not remove DistributedYamlImporter function for {}, as it isn't added", key);
        return false;
    }
    if (searchIt->second != pImportFn) {
        FOE_LOG(foeImexYaml, Warning,
                "Attempted to remove DistributedYamlImporter function for {}, but the provided "
                "function pointers are not the same as was added",
                key);
        return false;
    }

    mComponentFns.erase(searchIt);
    return true;
}