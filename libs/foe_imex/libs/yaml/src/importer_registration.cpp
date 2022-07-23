// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/yaml/importer_registration.h>

#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/importer.hpp>

#include "common.hpp"
#include "importer_registration.hpp"
#include "log.hpp"
#include "result.h"

#include <filesystem>

foeResult foeImexYamlCreateImporter(foeIdGroup group,
                                    char const *pFilesystemPath,
                                    foeImporterBase **ppImporter) {
    std::filesystem::path fsPath{pFilesystemPath};

    // Root Directory
    if (!std::filesystem::is_directory(fsPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY);

    // Dependencies File
    if (!std::filesystem::exists(fsPath / dependenciesFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / dependenciesFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE);

    // Resource Index Data File
    if (!std::filesystem::exists(fsPath / resourceIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / resourceIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE);

    // Entity Index Data File
    if (!std::filesystem::exists(fsPath / entityIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST);
    if (!std::filesystem::is_regular_file(fsPath / entityIndexDataFilePath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE);

    // Check optional directories (resources, components and external data), fail if they fail
    // Resources Directory
    if (std::filesystem::exists(fsPath / resourceDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / resourceDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY);

    // Entity Component Data Directory
    if (std::filesystem::exists(fsPath / entityDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / entityDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY);

    // External Data Directory
    if (std::filesystem::exists(fsPath / externalDirectoryPath) &&
        !std::filesystem::is_directory(fsPath / externalDirectoryPath))
        return to_foeResult(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY);

    *ppImporter = new foeYamlImporter{group, fsPath.string().c_str()};

    return to_foeResult(FOE_IMEX_YAML_SUCCESS);
}

extern "C" foeResult foeImexYamlRegisterImporter() {
    return foeImexRegisterImporter(&foeImexYamlCreateImporter);
}

extern "C" foeResult foeImexYamlDeregisterImporter() {
    return foeImexDeregisterImporter(&foeImexYamlCreateImporter);
}