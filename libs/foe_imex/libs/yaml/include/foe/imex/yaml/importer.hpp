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

#ifndef FOE_IMEX_YAML_IMPORTER_HPP
#define FOE_IMEX_YAML_IMPORTER_HPP

#include <foe/ecs/group_translator.hpp>
#include <foe/ecs/id.hpp>
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
    FOE_IMEX_YAML_EXPORT foeYamlImporter(foeYamlImporterGenerator *pGenerator,
                                         foeIdGroup group,
                                         std::filesystem::path rootDir);

    FOE_IMEX_YAML_EXPORT foeIdGroup group() const noexcept override;
    FOE_IMEX_YAML_EXPORT std::string name() const noexcept override;
    FOE_IMEX_YAML_EXPORT void setGroupTranslator(foeIdGroupTranslator &&groupTranslator) override;

    FOE_IMEX_YAML_EXPORT bool getDependencies(
        std::vector<foeIdGroupValueNameSet> &dependencies) override;
    FOE_IMEX_YAML_EXPORT bool getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) override;
    FOE_IMEX_YAML_EXPORT bool getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) override;
    FOE_IMEX_YAML_EXPORT bool importStateData(
        foeEditorNameMap *pEntityNameMap,
        std::vector<foeComponentPoolBase *> &componentPools) override;

    FOE_IMEX_YAML_EXPORT bool importResourceDefinitions(
        foeEditorNameMap *pNameMap,
        std::vector<foeResourceLoaderBase *> &resourceLoaders,
        std::vector<foeResourcePoolBase *> &resourcePools) override;
    FOE_IMEX_YAML_EXPORT foeResourceCreateInfoBase *getResource(foeId id) override;

    FOE_IMEX_YAML_EXPORT std::filesystem::path findExternalFile(
        std::filesystem::path externalFilePath) override;

  public:
    std::filesystem::path mRootDir;
    foeIdGroup mGroup;
    foeYamlImporterGenerator *mGenerator;

    foeIdGroupTranslator mGroupTranslator;
};

#endif // FOE_IMEX_YAML_IMPORTER_HPP