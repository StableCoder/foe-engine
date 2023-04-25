// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_BINARY_IMPORTER_H
#define FOE_IMEX_BINARY_IMPORTER_H

#include <foe/ecs/group_translator.h>
#include <foe/imex/binary/export.h>
#include <foe/imex/importer.h>
#include <foe/resource/create_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeSimulation foeSimulation;

typedef foeResultSet (*PFN_foeImexBinaryImportResource)(void const *,
                                                        uint32_t *,
                                                        foeEcsGroupTranslator,
                                                        foeResourceCreateInfo *);

typedef foeResultSet (*PFN_foeImexBinaryImportComponent)(
    void const *, uint32_t *, foeEcsGroupTranslator, foeEntityID, foeSimulation const *);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeCreateBinaryImporter(foeIdGroup group,
                                     char const *pFilePath,
                                     foeImexImporter *pImporter);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeImexBinaryRegisterResourceImportFns(char const *pBinaryKey,
                                                    PFN_foeImexBinaryImportResource importFn);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeImexBinaryDeregisterResourceImportFns(char const *pBinaryKey,
                                                      PFN_foeImexBinaryImportResource importFn);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeImexBinaryRegisterComponentImportFn(char const *pBinaryKey,
                                                    PFN_foeImexBinaryImportComponent importFn);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeImexBinaryDeregisterComponentImportFn(char const *pBinaryKey,
                                                      PFN_foeImexBinaryImportComponent importFn);

FOE_IMEX_BINARY_EXPORT
foeResultSet foeImexBinaryRegisterImporter();

FOE_IMEX_BINARY_EXPORT
void foeImexBinaryDeregisterImporter();

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_BINARY_IMPORTER_H