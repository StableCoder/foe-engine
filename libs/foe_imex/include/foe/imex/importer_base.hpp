// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_IMPORTER_BASE_HPP
#define FOE_IMEX_IMPORTER_BASE_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/ecs/indexes.h>
#include <foe/ecs/name_map.h>
#include <foe/error_code.h>
#include <foe/resource/create_info.h>

#include <filesystem>
#include <string>
#include <vector>

struct foeSimulation;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeIdGroup group() const noexcept = 0;
    virtual char const *name() const noexcept = 0;
    virtual void setGroupTranslator(foeEcsGroupTranslator groupTranslator) = 0;

    virtual foeResult getDependencies(uint32_t *pDependencyCount,
                                      foeIdGroup *pDependencyGroups,
                                      uint32_t *pNamesLength,
                                      char *pNames) = 0;
    virtual bool getGroupEntityIndexData(foeEcsIndexes indexes) = 0;
    virtual bool getGroupResourceIndexData(foeEcsIndexes indexes) = 0;
    virtual bool importStateData(foeEcsNameMap entityNameMap, foeSimulation const *pSimulation) = 0;

    virtual bool importResourceDefinitions(foeEcsNameMap nameMap,
                                           foeSimulation const *pSimulation) = 0;
    virtual std::string getResourceEditorName(foeResourceID resourceID) = 0;
    virtual foeResourceCreateInfo getResource(foeId id) = 0;

    virtual std::filesystem::path findExternalFile(std::filesystem::path externalFilePath) = 0;
};

#endif // FOE_IMEX_IMPORTER_BASE_HPP