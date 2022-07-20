// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_YAML_IMPORTER_HPP
#define FOE_IMEX_YAML_IMPORTER_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/importer_base.hpp>
#include <foe/imex/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

class foeYamlImporterGenerator;

class foeYamlImporter : public foeImporterBase {
  public:
    FOE_IMEX_YAML_EXPORT foeYamlImporter(foeIdGroup group, std::filesystem::path rootDir);
    FOE_IMEX_YAML_EXPORT ~foeYamlImporter();

    FOE_IMEX_YAML_EXPORT foeIdGroup group() const noexcept override;
    FOE_IMEX_YAML_EXPORT char const *name() const noexcept override;
    FOE_IMEX_YAML_EXPORT void setGroupTranslator(foeEcsGroupTranslator groupTranslator) override;

    FOE_IMEX_YAML_EXPORT foeResult getDependencies(uint32_t *pDependencyCount,
                                                   foeIdGroup *pDependencyGroups,
                                                   uint32_t *pNamesLength,
                                                   char *pNames) override;
    FOE_IMEX_YAML_EXPORT bool getGroupEntityIndexData(foeEcsIndexes indexes) override;
    FOE_IMEX_YAML_EXPORT bool getGroupResourceIndexData(foeEcsIndexes indexes) override;
    FOE_IMEX_YAML_EXPORT bool importStateData(foeEcsNameMap nameMap,
                                              foeSimulation const *pSimulation) override;

    FOE_IMEX_YAML_EXPORT bool importResourceDefinitions(foeEcsNameMap nameMap,
                                                        foeSimulation const *pSimulation) override;
    FOE_IMEX_YAML_EXPORT foeResult getResourceEditorName(foeIdIndex resourceIndexID,
                                                         uint32_t *pNameLength,
                                                         char *pName) override;
    FOE_IMEX_YAML_EXPORT foeResourceCreateInfo getResource(foeId id) override;

    FOE_IMEX_YAML_EXPORT foeResult findExternalFile(char const *pExternalFilePath,
                                                    uint32_t *pPathLength,
                                                    char *pPath) override;

  public:
    std::filesystem::path mRootDir;
    foeIdGroup mGroup;
    std::string mName;

    bool mHasTranslation{false};
    foeEcsGroupTranslator mGroupTranslator{FOE_NULL_HANDLE};
};

/// Imports the definition of a resource from a YAML node
using PFN_foeImexYamlResourceImport = void (*)(YAML::Node const &,
                                               foeEcsGroupTranslator,
                                               foeResourceCreateInfo *);

/// Creates a resource from a given CreateInfo definition
using PFN_foeImexYamlResourceCreate = foeResult (*)(foeResourceID,
                                                    foeResourceCreateInfo,
                                                    foeSimulation const *);

/// Imports component data from a YAML node
using PFN_foeImexYamlComponent = bool (*)(YAML::Node const &,
                                          foeEcsGroupTranslator,
                                          foeEntityID,
                                          foeSimulation const *);

/**
 * @brief Adds a string/function pointer pair to the importer map
 * @param key String key corresponding to the Yaml node key it parses
 * @param pImportFn Function that properly parses a given node of the given key
 * @param pCreateFn Function that creates a resource of the proper type based on the node
 * @return True if the key/function was added, false otherwise
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlRegisterResourceFns(std::string_view key,
                                                         PFN_foeImexYamlResourceImport pImportFn,
                                                         PFN_foeImexYamlResourceCreate pCreateFn);

/**
 * @brief Removes the given key/function pair from the importer map
 * @param key Yaml node key of the associated importer function
 * @param pImportFn Function that properly parses a given node of the given key
 * @param pCreateFn Function that creates a resource of the proper type based on the node
 * @return True if the pair was successfully removed, false otherwise.
 * @note Reasons for failure are recorded in the log
 * @todo Change to return appropriate error code
 */
FOE_IMEX_YAML_EXPORT bool foeImexYamlDeregisterResourceFns(std::string_view key,
                                                           PFN_foeImexYamlResourceImport pImportFn,
                                                           PFN_foeImexYamlResourceCreate pCreateFn);

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