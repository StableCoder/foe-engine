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

#ifndef FOE_IMEX_YAML_GENERATOR_HPP
#define FOE_IMEX_YAML_GENERATOR_HPP

#include <foe/ecs/id.hpp>
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <functional>
#include <map>
#include <vector>

struct foeIdGroupTranslator;
struct foeResourceCreateInfoBase;
struct foeResourceLoaderBase;
struct foeResourcePoolBase;
struct foeComponentPoolBase;

FOE_IMEX_YAML_EXPORT bool foeRegisterYamlImportGenerator();
FOE_IMEX_YAML_EXPORT void foeDeregisterYamlImportGenerator();

class FOE_IMEX_YAML_EXPORT foeYamlImporterGenerator : public foeImporterGenerator {
  public:
    using ImportFn = void (*)(YAML::Node const &,
                              foeIdGroupTranslator const *,
                              foeResourceCreateInfoBase **);

    using CreateFn = bool (*)(foeResourceID,
                              foeResourceCreateInfoBase *,
                              std::vector<foeResourceLoaderBase *> &,
                              std::vector<foeResourcePoolBase *> &);

    using ComponentImportFn = bool (*)(YAML::Node const &,
                                       foeIdGroupTranslator const *,
                                       foeEntityID,
                                       std::vector<foeComponentPoolBase *> &);

    auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * override;

    /**
     * @brief Adds a string/function pointer pair to the importer map
     * @param key String key corresponding to the Yaml node key it parses
     * @param pImportFn Function that properly parses a given node of the given key
     * @param pCreateFn Function that creates a resource of the proper type based on the node
     * @return True if the key/function was added, false otherwise
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool addImporter(std::string key, ImportFn pImportFn, CreateFn pCreateFn);

    /**
     * @brief Removes the given key/function pair from the importer map
     * @param key Yaml node key of the associated importer function
     * @param pImportFn Function that properly parses a given node of the given key
     * @param pCreateFn Function that creates a resource of the proper type based on the node
     * @return True if the pair was successfully removed, false otherwise.
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool removeImporter(std::string key, ImportFn pImportFn, CreateFn pCreateFn);

    /**
     * @brief Adds a string/function pointer pair to the importer map
     * @param key String key corresponding to the Yaml node key it parses
     * @param pImportFn Function that properly parses a given node of the given key
     * @return True if the key/function was added, false otherwise
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool addComponentImporter(std::string key, ComponentImportFn pImportFn);

    /**
     * @brief Removes the given key/function pair from the importer map
     * @param key Yaml node key of the associated importer function
     * @param pImportFn Function that properly parses a given node of the given key
     * @return True if the pair was successfully removed, false otherwise.
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool removeComponentImporter(std::string key, ComponentImportFn pImportFn);

  private:
    friend class foeYamlImporter;

    struct ResourceFunctions {
        ImportFn pImport;
        CreateFn pCreate;
    };

    std::map<std::string, ResourceFunctions> mResourceFns;
    std::map<std::string, ComponentImportFn> mComponentFns;
};

#endif // FOE_IMEX_YAML_GENERATOR_HPP