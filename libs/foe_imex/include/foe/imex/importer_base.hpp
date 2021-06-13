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

#ifndef FOE_IMEX_IMPORTER_BASE_HPP
#define FOE_IMEX_IMPORTER_BASE_HPP

#include <foe/ecs/id.hpp>

#include <filesystem>
#include <string>
#include <vector>

struct foeIdGroupTranslator;
struct foeIdGroupValueNameSet;
class foeIdIndexGenerator;
class foeEditorNameMap;
struct foeComponentPoolBase;
struct foeResourceLoaderBase;
struct foeResourcePoolBase;
struct foeResourceCreateInfoBase;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual void setGroupTranslator(foeIdGroupTranslator &&groupTranslator) = 0;

    virtual bool getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) = 0;
    virtual bool getGroupEntityIndexData(foeIdIndexGenerator &ecsGroup) = 0;
    virtual bool getGroupResourceIndexData(foeIdIndexGenerator &ecsGroup) = 0;
    virtual bool importStateData(foeEditorNameMap *pEntityNameMap,
                                 std::vector<foeComponentPoolBase *> &componentPools) = 0;

    virtual bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                           std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                           std::vector<foeResourcePoolBase *> &resourcePools) = 0;
    virtual foeResourceCreateInfoBase *getResource(foeId id) = 0;

    virtual std::filesystem::path findExternalFile(std::filesystem::path externalFilePath) = 0;
};

#endif // FOE_IMEX_IMPORTER_BASE_HPP