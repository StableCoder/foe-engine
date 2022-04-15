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

#ifndef FOE_IMEX_YAML_GENERATOR_HPP
#define FOE_IMEX_YAML_GENERATOR_HPP

#include <foe/ecs/id.h>
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/export.h>
#include <foe/resource/create_info.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string_view>
#include <system_error>
#include <vector>

struct foeIdGroupTranslator;
struct foeSimulation;
struct foeSimulationLoaderData;
struct foeComponentPoolBase;

class FOE_IMEX_YAML_EXPORT foeYamlImporterGenerator : public foeImporterGenerator {
  public:
    auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * override;

  private:
    friend class foeYamlImporter;
};

#endif // FOE_IMEX_YAML_GENERATOR_HPP