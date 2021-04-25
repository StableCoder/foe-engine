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

#ifndef IMPORTER_BASE_HPP
#define IMPORTER_BASE_HPP

#include <foe/ecs/id.hpp>

#include <string>
#include <vector>

class foeEditorNameMap;
class foeIdIndexGenerator;
struct StatePools;
struct ResourcePools;
struct ResourceLoaders;
struct foeResourceCreateInfoBase;
struct foeIdGroupTranslator;
struct foeIdGroupValueNameSet;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual void setGroupTranslator(foeIdGroupTranslator &&groupTranslator) = 0;

    virtual bool getDependencies(std::vector<foeIdGroupValueNameSet> &dependencies) = 0;
    virtual bool getGroupIndexData(foeIdIndexGenerator &ecsGroup) = 0;
    virtual bool importStateData(StatePools *pStatePools) = 0;

    virtual bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                           ResourcePools *pResourcePools,
                                           ResourceLoaders *pResourceLoaders) = 0;
    virtual foeResourceCreateInfoBase *getResource(foeId id) = 0;
};

#endif // IMPORTER_BASE_HPP