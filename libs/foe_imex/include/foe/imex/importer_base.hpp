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

struct foeSimulation;

class foeImporterBase {
  public:
    virtual ~foeImporterBase() = default;

    virtual foeResult group(foeIdGroup *pGroup) const noexcept = 0;
    virtual foeResult name(char const **ppGroupName) const noexcept = 0;
    virtual foeResult setGroupTranslator(foeEcsGroupTranslator groupTranslator) = 0;

    virtual foeResult getDependencies(uint32_t *pDependencyCount,
                                      foeIdGroup *pDependencyGroups,
                                      uint32_t *pNamesLength,
                                      char *pNames) = 0;
    virtual foeResult getGroupEntityIndexData(foeEcsIndexes indexes) = 0;
    virtual foeResult getGroupResourceIndexData(foeEcsIndexes indexes) = 0;
    virtual foeResult importStateData(foeEcsNameMap entityNameMap,
                                      foeSimulation const *pSimulation) = 0;

    virtual foeResult importResourceDefinitions(foeEcsNameMap nameMap,
                                                foeSimulation const *pSimulation) = 0;
    virtual foeResult getResourceEditorName(foeResourceID resourceID,
                                            uint32_t *pNameLength,
                                            char *pName) = 0;
    virtual foeResult getResource(foeId id, foeResourceCreateInfo *pResourceCreateInfo) = 0;

    virtual foeResult findExternalFile(char const *pExternalFilePath,
                                       uint32_t *pPathLength,
                                       char *pPath) = 0;
};

#endif // FOE_IMEX_IMPORTER_BASE_HPP