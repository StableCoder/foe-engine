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

#ifndef FOE_IMEX_IMPORTER_BASE_HPP
#define FOE_IMEX_IMPORTER_BASE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/ecs/indexes.h>
#include <foe/error_code.h>
#include <foe/resource/create_info.h>

#include <filesystem>
#include <string>
#include <vector>

struct foeEditorNameMap;

struct foeSimulation;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual void setGroupTranslator(foeEcsGroupTranslator groupTranslator) = 0;

    virtual foeResult getDependencies(uint32_t *pDependencyCount,
                                      foeIdGroup *pDependencyGroups,
                                      uint32_t *pNamesLength,
                                      char *pNames) = 0;
    virtual bool getGroupEntityIndexData(foeEcsIndexes indexes) = 0;
    virtual bool getGroupResourceIndexData(foeEcsIndexes indexes) = 0;
    virtual bool importStateData(foeEditorNameMap *pEntityNameMap,
                                 foeSimulation const *pSimulation) = 0;

    virtual bool importResourceDefinitions(foeEditorNameMap *pNameMap,
                                           foeSimulation const *pSimulation) = 0;
    virtual std::string getResourceEditorName(foeResourceID resourceID) = 0;
    virtual foeResourceCreateInfo getResource(foeId id) = 0;

    virtual std::filesystem::path findExternalFile(std::filesystem::path externalFilePath) = 0;
};

#endif // FOE_IMEX_IMPORTER_BASE_HPP