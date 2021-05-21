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

#ifndef DISTRIBUTED_YAML_GENERATOR_HPP
#define DISTRIBUTED_YAML_GENERATOR_HPP

#include <foe/ecs/id.hpp>
#include <foe/imex/importers.hpp>
#include <foe/resource/create_info_base.hpp>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <functional>
#include <map>

struct foeIdGroupTranslator;

class foeDistributedYamlImporterGenerator : public foeImporterGenerator {
  public:
    using ImportFunc = void (*)(YAML::Node const &,
                                foeIdGroupTranslator const *,
                                foeResourceCreateInfoBase **);

    auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * override;

    /**
     * @brief Adds a string/function pointer pair to the importer map
     * @param key String key corresponding to the Yaml node key it parses
     * @param pFunction Function that properly parses a given node
     * @return True if the key/function was added, false otherwise
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool addImporter(std::string key, ImportFunc pFunction);

    /**
     * @brief Removes the given key/function pair from the importer map
     * @param key Yaml node key of the associated importer function
     * @param pFunction Function pointer that is to be removed
     * @return True if the pair was successfully removed, false otherwise.
     * @note Reasons for failure are recorded in the log
     * @todo Change to return appropriate error code
     */
    bool removeImporter(std::string key, ImportFunc pFunction);

  private:
    friend class foeDistributedYamlImporter;

    std::map<std::string, ImportFunc> mImportFunctions;
};

#endif // DISTRIBUTED_YAML_GENERATOR_HPP