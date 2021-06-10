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

#ifndef FOE_IMEX_YAML_EXPORTER_HPP
#define FOE_IMEX_YAML_EXPORTER_HPP

#include <foe/ecs/id.hpp>
#include <foe/imex/exporter_base.hpp>
#include <foe/imex/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <shared_mutex>
#include <string>
#include <vector>

struct foeSimulationState;
struct foeResourcePoolBase;
struct foeComponentPoolBase;

struct foeKeyYamlPair {
    std::string key;
    YAML::Node data;
};

class FOE_IMEX_YAML_EXPORT foeYamlExporter : public foeExporterBase {
  public:
    foeYamlExporter();
    ~foeYamlExporter();

    foeYamlExporter(foeYamlExporter const &) = delete;
    foeYamlExporter(foeYamlExporter &&) = delete;

    foeYamlExporter operator=(foeYamlExporter const &) = delete;
    foeYamlExporter operator=(foeYamlExporter &&) = delete;

    bool exportState(std::filesystem::path path, foeSimulationState *pSimState);

    bool registerResourceFn(std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID,
                                                                       foeResourcePoolBase **,
                                                                       uint32_t));
    void deregisterResourceFn(std::vector<foeKeyYamlPair> (*pResourceFn)(foeResourceID,
                                                                         foeResourcePoolBase **,
                                                                         uint32_t));

    bool registerComponentFn(std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID,
                                                                         foeComponentPoolBase **,
                                                                         uint32_t));
    void deregisterComponentFn(std::vector<foeKeyYamlPair> (*pComponentFn)(foeEntityID,
                                                                           foeComponentPoolBase **,
                                                                           uint32_t));

  private:
    bool exportDependencies(std::filesystem::path rootOutPath, foeSimulationState *pSimState);
    bool exportGroupResourceIndexData(std::filesystem::path rootOutPath,
                                      foeSimulationState *pSimState);
    bool exportGroupEntityIndexData(std::filesystem::path rootOutPath,
                                    foeSimulationState *pSimState);

    bool exportResources(std::filesystem::path rootOutPath,
                         foeIdGroup group,
                         foeSimulationState *pSimState);
    bool exportComponentData(std::filesystem::path rootOutPath,
                             foeIdGroup group,
                             foeSimulationState *pSimState);

    std::shared_mutex mSync;

    std::vector<std::vector<foeKeyYamlPair> (*)(foeResourceID, foeResourcePoolBase **, uint32_t)>
        mResourceFns;
    std::vector<std::vector<foeKeyYamlPair> (*)(foeEntityID, foeComponentPoolBase **, uint32_t)>
        mComponentFns;
};

#endif // FOE_IMEX_YAML_EXPORTER_HPP