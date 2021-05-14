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

#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/id.hpp>
#include <foe/resource/create_info_base.hpp>
#include <yaml-cpp/yaml.h>

#include "../state_import/importer_base.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

class foeDistributedYamlImporterGenerator;

class foeDistributedYamlImporter : public foeImporterBase {
  public:
    foeDistributedYamlImporter(foeDistributedYamlImporterGenerator *pGenerator,
                               foeIdGroup group,
                               std::filesystem::path rootDir);

    foeIdGroup group() const noexcept override;
    std::string name() const noexcept override;
    void setGroupTranslator(foeIdGroupTranslator &&groupTranslator) override;

    bool getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) override;
    bool getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) override;
    bool getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) override;
    bool importStateData(StatePools *pStatePools) override;

    bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                   ResourcePools *pResourcePools,
                                   ResourceLoaders *pResourceLoaders) override;
    foeResourceCreateInfoBase *getResource(foeId id) override;

  public:
    std::filesystem::path mRootDir;
    foeIdGroup mGroup;
    foeDistributedYamlImporterGenerator *mGenerator;

    foeIdGroupTranslator mGroupTranslator;
};