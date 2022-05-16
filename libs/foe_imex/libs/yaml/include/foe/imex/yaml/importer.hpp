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

#ifndef FOE_IMEX_YAML_IMPORTER_HPP
#define FOE_IMEX_YAML_IMPORTER_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
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

    FOE_IMEX_YAML_EXPORT foeIdGroup group() const noexcept override;
    FOE_IMEX_YAML_EXPORT std::string name() const noexcept override;
    FOE_IMEX_YAML_EXPORT void setGroupTranslator(foeEcsGroupTranslator groupTranslator) override;

    FOE_IMEX_YAML_EXPORT foeErrorCode getDependencies(uint32_t *pDependencyCount,
                                                      foeIdGroup *pDependencyGroups,
                                                      uint32_t *pNamesLength,
                                                      char *pNames) override;
    FOE_IMEX_YAML_EXPORT bool getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) override;
    FOE_IMEX_YAML_EXPORT bool getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) override;
    FOE_IMEX_YAML_EXPORT bool importStateData(foeEditorNameMap *pEntityNameMap,
                                              foeSimulation const *pSimulation) override;

    FOE_IMEX_YAML_EXPORT bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                                        foeSimulation const *pSimulation) override;
    FOE_IMEX_YAML_EXPORT std::string getResourceEditorName(foeIdIndex resourceIndexID) override;
    FOE_IMEX_YAML_EXPORT foeResourceCreateInfo getResource(foeId id) override;

    FOE_IMEX_YAML_EXPORT std::filesystem::path findExternalFile(
        std::filesystem::path externalFilePath) override;

  public:
    std::filesystem::path mRootDir;
    foeIdGroup mGroup;

    bool mHasTranslation{false};
    foeEcsGroupTranslator mGroupTranslator{FOE_NULL_HANDLE};
};

/// Imports the definition of a resource from a YAML node
using PFN_foeImexYamlResourceImport = void (*)(YAML::Node const &,
                                               foeEcsGroupTranslator,
                                               foeResourceCreateInfo *);

/// Creates a resource from a given CreateInfo definition
using PFN_foeImexYamlResourceCreate = std::error_code (*)(foeResourceID,
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