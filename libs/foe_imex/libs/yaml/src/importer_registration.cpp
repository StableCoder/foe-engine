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

#include <foe/imex/yaml/importer_registration.h>

#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/importer.hpp>

#include "common.hpp"
#include "error_code.hpp"
#include "log.hpp"

#include <filesystem>

namespace {

foeErrorCode createYamlImporter(foeIdGroup group,
                                char const *pFilesystemPath,
                                foeImporterBase **ppImporter) {
    std::filesystem::path fsPath{pFilesystemPath};

    // Root Directory
    if (!std::filesystem::is_directory(fsPath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY);

    // Dependencies File
    if (!std::filesystem::exists(fsPath / dependenciesFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / dependenciesFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE);

    // Resource Index Data File
    if (!std::filesystem::exists(fsPath / resourceIndexDataFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / resourceIndexDataFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE);

    // Entity Index Data File
    if (!std::filesystem::exists(fsPath / entityIndexDataFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / entityIndexDataFilePath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE);

    // Check optional directories (resources, components and external data), fail if they fail
    // Resources Directory
    if (std::filesystem::exists(fsPath / resourceDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / resourceDirectoryPath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY);

    // Entity Component Data Directory
    if (std::filesystem::exists(fsPath / entityDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / entityDirectoryPath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY);

    // External Data Directory
    if (std::filesystem::exists(fsPath / externalDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / externalDirectoryPath))
        return foeToErrorCode(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY);

    *ppImporter = new foeYamlImporter{group, fsPath};

    return foeToErrorCode(FOE_IMEX_YAML_SUCCESS);
}

} // namespace

extern "C" foeErrorCode foeImexYamlRegisterImporter() {
    return foeToErrorCode(foeImexRegisterImporter(&createYamlImporter));
}

extern "C" foeErrorCode foeImexYamlDeregisterImporter() {
    return foeToErrorCode(foeImexDeregisterImporter(&createYamlImporter));
}