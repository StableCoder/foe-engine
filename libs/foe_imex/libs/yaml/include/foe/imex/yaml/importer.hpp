// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_YAML_IMPORTER_HPP
#define FOE_IMEX_YAML_IMPORTER_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/imex/importer.h>
#include <foe/imex/yaml/export.h>
#include <foe/result.h>
#include <yaml-cpp/yaml.h>

#include <string_view>

FOE_IMEX_YAML_EXPORT foeResultSet foeCreateYamlImporter(foeIdGroup group,
                                                        char const *pRootDir,
                                                        foeImexImporter *pImporter);

/// Imports the definition of a resource from a YAML node
using PFN_foeImexYamlResourceImport = void (*)(YAML::Node const &,
                                               foeEcsGroupTranslator,
                                               foeResourceCreateInfo *);

/// Imports component data from a YAML node
using PFN_foeImexYamlComponent = bool (*)(YAML::Node const &,
                                          foeEcsGroupTranslator,
                                          foeEntityID,
                                          foeSimulation const *);

/**
 * @brief Adds a string/function pointer pair to the importer map
 * @param key String key corresponding to the Yaml node key it parses
 * @param pImportFn Function that properly parses a given node of the given key
 * @return True if the key/function was added, false otherwise
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlRegisterResourceFns(std::string_view key,
                                                         PFN_foeImexYamlResourceImport pImportFn);

/**
 * @brief Removes the given key/function pair from the importer map
 * @param key Yaml node key of the associated importer function
 * @param pImportFn Function that properly parses a given node of the given key
 * @return True if the pair was successfully removed, false otherwise.
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlDeregisterResourceFns(std::string_view key,
                                                           PFN_foeImexYamlResourceImport pImportFn);

/**
 * @brief Adds a string/function pointer pair to the importer map
 * @param key String key corresponding to the Yaml node key it parses
 * @param pImportFn Function that properly parses a given node of the given key
 * @return True if the key/function was added, false otherwise
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlRegisterComponentFn(std::string_view key,
                                                         PFN_foeImexYamlComponent pImportFn);

/**
 * @brief Removes the given key/function pair from the importer map
 * @param key Yaml node key of the associated importer function
 * @param pImportFn Function that properly parses a given node of the given key
 * @return True if the pair was successfully removed, false otherwise.
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlDeregisterComponentFn(std::string_view key,
                                                           PFN_foeImexYamlComponent pImportFn);

#endif // FOE_IMEX_YAML_IMPORTER_HPP