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

#include "distributed_yaml_generator.hpp"

#include <foe/log.hpp>

#include "distributed_yaml.hpp"

namespace {

constexpr std::string_view dependenciesFilePath = "dependencies.yml";
constexpr std::string_view indexDataFilePath = "index_data.yml";
constexpr std::string_view resourcesDirectoryPath = "resources";
constexpr std::string_view stateDirectoryPath = "state";

} // namespace

auto foeDistributedYamlImporterGenerator::createImporter(foeIdGroup group,
                                                         std::filesystem::path stateDataPath)
    -> foeImporterBase * {
    if (std::filesystem::is_directory(stateDataPath) &&
        // Dependencies
        (std::filesystem::exists(stateDataPath / dependenciesFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / dependenciesFilePath)) &&
        // Index Data
        (std::filesystem::exists(stateDataPath / indexDataFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / indexDataFilePath)) &&
        // Resources
        (std::filesystem::exists(stateDataPath / resourcesDirectoryPath) &&
         std::filesystem::is_directory(stateDataPath / resourcesDirectoryPath)) &&
        // State
        (std::filesystem::exists(stateDataPath / stateDirectoryPath) &&
         std::filesystem::is_directory(stateDataPath / stateDirectoryPath))) {
        return new foeDistributedYamlImporter{this, group, stateDataPath};
    }

    return nullptr;
}

bool foeDistributedYamlImporterGenerator::addImporter(std::string type,
                                                      uint32_t version,
                                                      ImportFunc function) {
    auto key = std::make_tuple(type, version);

    auto searchIt = mImportFunctions.find(key);
    if (searchIt != mImportFunctions.end()) {
        FOE_LOG(General, Error,
                "Could not add DistributedYamlImporter function for {}:{}, as it already exists",
                type, version);
        return false;
    }

    FOE_LOG(General, Info, "Adding DistributedYamlImporter function for {}:{}", type, version);
    mImportFunctions[key] = function;
    return true;
}

bool foeDistributedYamlImporterGenerator::removeImporter(std::string type, uint32_t version) {
    auto key = std::make_tuple(type, version);

    auto searchIt = mImportFunctions.find(key);
    if (searchIt == mImportFunctions.end()) {
        FOE_LOG(General, Error,
                "Could not remove DistributedYamlImporter function for {}:{}, as it isn't added",
                type, version);
        return false;
    }

    mImportFunctions.erase(searchIt);
    return true;
}