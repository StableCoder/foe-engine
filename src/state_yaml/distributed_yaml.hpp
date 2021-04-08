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

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

class foeDistributedYamlImporter {
  public:
    using ImportFunc = std::function<void(YAML::Node const &, foeResourceCreateInfoBase **)>;

    foeDistributedYamlImporter(std::filesystem::path rootDir);

    bool addImporter(std::string type, uint32_t version, ImportFunc function);
    bool removeImporter(std::string type, uint32_t version);
    bool getResource(foeId id, foeResourceCreateInfoBase **ppCreateInfo);

  public:
    static constexpr std::string_view cResourceSubDir = "resources";
    static constexpr std::string_view cStateSubDir = "state_data";

    std::filesystem::path mRootDir;

    using ImportKey = std::tuple<std::string, uint32_t>;

    std::map<ImportKey, ImportFunc> mImportFunctions;
};