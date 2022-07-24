// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_EXPORTERS_H
#define FOE_IMEX_EXPORTERS_H

#include <foe/ecs/id.h>
#include <foe/imex/export.h>
#include <foe/result.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeSimulation;

#define FOE_EXPORTER_VERSION(major, minor, patch)                                                  \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

typedef uint32_t foeExporterVersion;

typedef struct foeExporter {
    char const *pName;
    foeExporterVersion version;
    foeResultSet (*pExportFn)(char const *, struct foeSimulation *);
} foeExporter;

FOE_IMEX_EXPORT bool foeCompareExporters(foeExporter const *lhs, foeExporter const *rhs);

FOE_IMEX_EXPORT foeResultSet foeImexRegisterExporter(foeExporter exporter);
FOE_IMEX_EXPORT foeResultSet foeImexDeregisterExporter(foeExporter exporter);

FOE_IMEX_EXPORT void foeImexGetExporters(uint32_t *pExporterCount, foeExporter *pExporters);

typedef struct foeExportFunctionality {
    foeResultSet (*onRegister)(foeExporter);
    void (*onDeregister)(foeExporter);
} foeExportFunctionality;

FOE_IMEX_EXPORT foeResultSet
foeRegisterExportFunctionality(foeExportFunctionality const *functionality);
FOE_IMEX_EXPORT void foeDeregisterExportFunctionality(foeExportFunctionality const *functionality);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_EXPORTERS_H