// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/importer.h>

#include <foe/imex/type_defs.h>

#include "result.h"

FOE_DEFINE_STATIC_HANDLE_CASTS(importer, foeBaseInStructure, foeImexImporter)

static void *findStruct(foeBaseInStructure const *pStruct, foeStructureType sType) {
    for (; pStruct != NULL; pStruct = pStruct->pNext) {
        if (pStruct->sType == sType) {
            return (void *)pStruct;
        }
    }

    return NULL;
}

void foeDestroyImporter(foeImexImporter importer) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL && pBaseFns->destroyImporter != NULL)
        return pBaseFns->destroyImporter(importer);
}

foeResultSet foeImexImporterGetGroupID(foeImexImporter importer, foeIdGroup *pGroup) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getGroupID != NULL) {
            return pBaseFns->getGroupID(importer, pGroup);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetGroupName(foeImexImporter importer, char const **pGroupName) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getGroupName != NULL) {
            return pBaseFns->getGroupName(importer, pGroupName);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterSetGroupTranslator(foeImexImporter importer,
                                               foeEcsGroupTranslator groupTranslator) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->setGroupTranslator != NULL) {
            return pBaseFns->setGroupTranslator(importer, groupTranslator);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetDependencies(foeImexImporter importer,
                                            uint32_t *pDependencyCount,
                                            foeIdGroup *pDependencyGroups,
                                            uint32_t *pNamesLength,
                                            char *pNames) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getDependencies != NULL) {
            return pBaseFns->getDependencies(importer, pDependencyCount, pDependencyGroups,
                                             pNamesLength, pNames);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetGroupEntityIndexData(foeImexImporter importer,
                                                    foeEcsIndexes indexes) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getGroupEntityIndexData != NULL) {
            return pBaseFns->getGroupEntityIndexData(importer, indexes);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetGroupResourceIndexData(foeImexImporter importer,
                                                      foeEcsIndexes indexes) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getGroupResourceIndexData != NULL) {
            return pBaseFns->getGroupResourceIndexData(importer, indexes);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetStateData(foeImexImporter importer,
                                         foeEcsNameMap entityNameMap,
                                         struct foeSimulation const *pSimulation) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->importStateData != NULL) {
            return pBaseFns->importStateData(importer, entityNameMap, pSimulation);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetResourceDefinitions(foeImexImporter importer,
                                                   foeEcsNameMap resourceNameMap,
                                                   struct foeSimulation const *pSimulation) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->importResourceDefinitions != NULL) {
            return pBaseFns->importResourceDefinitions(importer, resourceNameMap, pSimulation);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetResourceEditorName(foeImexImporter importer,
                                                  foeResourceID resourceID,
                                                  uint32_t *pNameLength,
                                                  char *pName) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getResourceEditorName != NULL) {
            return pBaseFns->getResourceEditorName(importer, resourceID, pNameLength, pName);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterGetResourceCreateInfo(foeImexImporter importer,
                                                  foeId resourceID,
                                                  foeResourceCreateInfo *pResourceCreateInfo) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->getResourceCreateInfo != NULL) {
            return pBaseFns->getResourceCreateInfo(importer, resourceID, pResourceCreateInfo);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}

foeResultSet foeImexImporterFindExternalFile(foeImexImporter importer,
                                             char const *pExternalFilePath,
                                             foeManagedMemory *pManagedMemory) {
    foeImexImporterCalls const *pBaseFns =
        findStruct(importer_from_handle(importer), FOE_IMEX_STRUCTURE_TYPE_IMPORTER_CALLS);

    if (pBaseFns != NULL) {
        if (pBaseFns->findExternalFile != NULL) {
            return pBaseFns->findExternalFile(importer, pExternalFilePath, pManagedMemory);
        } else {
            return to_foeResult(FOE_IMEX_ERROR_FUNCTION_NOT_DEFINED);
        }
    }

    return to_foeResult(FOE_IMEX_ERROR_STRUCTURE_NOT_FOUND);
}
