// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_BINARY_EXPORTER_H
#define FOE_IMEX_BINARY_EXPORTER_H

#include <foe/ecs/id.h>
#include <foe/imex/binary/export.h>
#include <foe/resource/create_info.h>
#include <foe/result.h>

#include <stdint.h>

typedef struct foeSimulation foeSimulation;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeImexBinarySet {
    char const *pKey;
    void *pData;
    uint32_t dataSize;
} foeImexBinarySet;

typedef struct foeImexBinaryFiles {
    char const **ppFiles;
    uint32_t fileCount;
} foeImexBinaryFiles;

typedef foeResultSet (*PFN_foeImexBinaryExportResource)(foeResourceCreateInfo,
                                                        foeImexBinarySet *,
                                                        foeImexBinaryFiles *);

typedef foeResultSet (*PFN_foeImexBinaryExportComponent)(foeEntityID,
                                                         foeSimulation const *,
                                                         foeImexBinarySet *);

FOE_IMEX_BINARY_EXPORT foeResultSet
foeImexBinaryRegisterResourceExportFn(PFN_foeImexBinaryExportResource exportResourceFn);

FOE_IMEX_BINARY_EXPORT foeResultSet
foeImexBinaryDeregisterResourceExportFn(PFN_foeImexBinaryExportResource exportResourceFn);

FOE_IMEX_BINARY_EXPORT foeResultSet
foeImexBinaryRegisterComponentExportFn(PFN_foeImexBinaryExportComponent exportComponentFn);

FOE_IMEX_BINARY_EXPORT foeResultSet
foeImexBinaryDeregisterComponentExportFn(PFN_foeImexBinaryExportComponent exportComponentFn);

FOE_IMEX_BINARY_EXPORT foeResultSet foeImexBinaryRegisterExporter();

FOE_IMEX_BINARY_EXPORT void foeImexBinaryDeregisterExporter();

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_BINARY_EXPORTER_H