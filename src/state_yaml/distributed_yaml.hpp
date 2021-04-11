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

#include <foe/ecs/id.hpp>
#include <foe/resource/create_info_base.hpp>
#include <yaml-cpp/yaml.h>

#include "../state_import/importer_base.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

auto createDistributedYamlImporter(foeIdGroup group, std::filesystem::path stateDataPath)
    -> foeImporterBase *;

class foeDistributedYamlImporter : public foeImporterBase {
  public:
    using ImportFunc = std::function<void(YAML::Node const &, foeResourceCreateInfoBase **)>;

    foeDistributedYamlImporter(foeIdGroup group, std::filesystem::path rootDir);

    foeIdGroup group() const noexcept override;
    std::string name() const noexcept override;
    void setGroupTranslation(foeGroupTranslation &&groupTranslation) override;

    bool getDependencies(std::vector<std::string> &dependencies) override;
    bool getGroupIndexData(foeIdIndexGenerator &ecsGroup) override;
    bool importStateData(foeEcsGroups *pGroups, StatePools *pStatePools) override;

    bool addImporter(std::string type, uint32_t version, ImportFunc function);
    bool removeImporter(std::string type, uint32_t version);
    bool getResource(foeId id, foeResourceCreateInfoBase **ppCreateInfo);

  public:
    static constexpr std::string_view cResourceSubDir = "resources";
    static constexpr std::string_view cStateSubDir = "state_data";

    std::filesystem::path mRootDir;

    using ImportKey = std::tuple<std::string, uint32_t>;

    foeIdGroup mGroup;
    foeGroupTranslation mGroupTranslation;
    std::map<ImportKey, ImportFunc> mImportFunctions;
};