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
#include <foe/resource/create_info_base.hpp>
#include <yaml-cpp/yaml.h>

#include "../state_import/importers.hpp"

#include <filesystem>
#include <functional>
#include <map>

class foeDistributedYamlImporterGenerator : public foeImporterGenerator {
  public:
    using ImportFunc = std::function<void(YAML::Node const &, foeResourceCreateInfoBase **)>;

    auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * override;

    bool addImporter(std::string type, uint32_t version, ImportFunc function);
    bool removeImporter(std::string type, uint32_t version);

  private:
    friend class foeDistributedYamlImporter;

    using ImportKey = std::tuple<std::string, uint32_t>;

    std::map<ImportKey, ImportFunc> mImportFunctions;
};

#endif // DISTRIBUTED_YAML_GENERATOR_HPP