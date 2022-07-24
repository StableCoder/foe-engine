// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_IMPORTER_HPP
#define FOE_IMEX_IMPORTER_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/ecs/indexes.h>
#include <foe/ecs/name_map.h>
#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/imex/export.h>
#include <foe/resource/create_info.h>
#include <foe/type_defs.h>

FOE_DEFINE_HANDLE(foeImexImporter)

struct foeSimulation;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeImexImporterCalls {
    foeStructureType sType;
    void *pNext;

    void (*destroyImporter)(foeImexImporter);

    foeResult (*getGroupID)(foeImexImporter, foeIdGroup *);
    foeResult (*getGroupName)(foeImexImporter, char const **);
    foeResult (*setGroupTranslator)(foeImexImporter, foeEcsGroupTranslator);

    foeResult (*getDependencies)(foeImexImporter, uint32_t *, foeIdGroup *, uint32_t *, char *);
    foeResult (*getGroupEntityIndexData)(foeImexImporter, foeEcsIndexes);
    foeResult (*getGroupResourceIndexData)(foeImexImporter, foeEcsIndexes);
    foeResult (*importStateData)(foeImexImporter, foeEcsNameMap, struct foeSimulation const *);

    foeResult (*importResourceDefinitions)(foeImexImporter,
                                           foeEcsNameMap,
                                           struct foeSimulation const *);
    foeResult (*getResourceEditorName)(foeImexImporter, foeResourceID, uint32_t *, char *);
    foeResult (*getResourceCreateInfo)(foeImexImporter, foeResourceID, foeResourceCreateInfo *);
    foeResult (*findExternalFile)(foeImexImporter, char const *, uint32_t *, char *);
} foeImexImporterCalls;

FOE_IMEX_EXPORT void foeDestroyImporter(foeImexImporter importer);

FOE_IMEX_EXPORT foeResult foeImexImporterGetGroupID(foeImexImporter importer, foeIdGroup *pGroup);

FOE_IMEX_EXPORT foeResult foeImexImporterGetGroupName(foeImexImporter importer,
                                                      char const **ppGroupName);

FOE_IMEX_EXPORT foeResult foeImexImporterSetGroupTranslator(foeImexImporter importer,
                                                            foeEcsGroupTranslator groupTranslator);

FOE_IMEX_EXPORT foeResult foeImexImporterGetDependencies(foeImexImporter importer,
                                                         uint32_t *pDependencyCount,
                                                         foeIdGroup *pDependencyGroups,
                                                         uint32_t *pNamesLength,
                                                         char *pNames);

FOE_IMEX_EXPORT foeResult foeImexImporterGetGroupEntityIndexData(foeImexImporter importer,
                                                                 foeEcsIndexes indexes);

FOE_IMEX_EXPORT foeResult foeImexImporterGetGroupResourceIndexData(foeImexImporter importer,
                                                                   foeEcsIndexes indexes);

FOE_IMEX_EXPORT foeResult foeImexImporterGetStateData(foeImexImporter importer,
                                                      foeEcsNameMap entityNameMap,
                                                      struct foeSimulation const *pSimulation);

FOE_IMEX_EXPORT foeResult
foeImexImporterGetResourceDefinitions(foeImexImporter importer,
                                      foeEcsNameMap resourceNameMap,
                                      struct foeSimulation const *pSimulation);

FOE_IMEX_EXPORT foeResult foeImexImporterGetResourceEditorName(foeImexImporter importer,
                                                               foeResourceID resourceID,
                                                               uint32_t *pNameLength,
                                                               char *pName);

FOE_IMEX_EXPORT foeResult foeImexImporterGetResourceCreateInfo(
    foeImexImporter importer, foeId resourceID, foeResourceCreateInfo *pResourceCreateInfo);

FOE_IMEX_EXPORT foeResult foeImexImporterFindExternalFile(foeImexImporter importer,
                                                          char const *pExternalFilePath,
                                                          uint32_t *pPathLength,
                                                          char *pPath);

typedef foeResult (*PFN_foeImexCreateImporter)(foeIdGroup, char const *, foeImexImporter *);

FOE_IMEX_EXPORT foeResult foeImexRegisterImporter(PFN_foeImexCreateImporter createImporter);
FOE_IMEX_EXPORT foeResult foeImexDeregisterImporter(PFN_foeImexCreateImporter createImporter);

FOE_IMEX_EXPORT foeImexImporter createImporter(foeIdGroup group, char const *pPath);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_IMPORTER_HPP