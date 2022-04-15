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

#include <foe/imex/yaml/generator.hpp>
#include <foe/imex/yaml/importer_registration.h>

#include <foe/imex/yaml/importer.hpp>

#include "common.hpp"
#include "log.hpp"

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
        (std::filesystem::exists(stateDataPath / entityIndexDataFilePath) &&
         std::filesystem::is_regular_file(stateDataPath / entityIndexDataFilePath))) {

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
        if (std::filesystem::exists(stateDataPath / entityDirectoryPath) &&
            !std::filesystem::is_directory(stateDataPath / entityDirectoryPath))
            return nullptr;

        return new foeYamlImporter{this, group, stateDataPath};
    }

    return nullptr;
}

namespace {
foeYamlImporterGenerator gGenerator;
}

extern "C" foeErrorCode foeRegisterYamlImportGenerator() {
    std::error_code errC = foeRegisterImportGenerator(&gGenerator);

    return foeToErrorCode(errC);
}

extern "C" foeErrorCode foeDeregisterYamlImportGenerator() {
    std::error_code errC = foeDeregisterImportGenerator(&gGenerator);

    return foeToErrorCode(errC);
}