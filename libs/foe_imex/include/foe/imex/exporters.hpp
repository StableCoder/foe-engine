// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_EXPORTERS_HPP
#define FOE_IMEX_EXPORTERS_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/export.h>

struct foeSimulation;

struct foeExporterVersion {
    unsigned int major : 10;
    unsigned int minor : 10;
    unsigned int patch : 12;
};

FOE_IMEX_EXPORT bool operator==(foeExporterVersion const &lhs, foeExporterVersion const &rhs);
FOE_IMEX_EXPORT bool operator!=(foeExporterVersion const &lhs, foeExporterVersion const &rhs);

static_assert(sizeof(foeExporterVersion) == sizeof(unsigned int));

struct foeExporter {
    char const *pName;
    foeExporterVersion version;
    foeResult (*pExportFn)(char const *, foeSimulation *);
};

FOE_IMEX_EXPORT bool operator==(foeExporter const &lhs, foeExporter const &rhs);
FOE_IMEX_EXPORT bool operator!=(foeExporter const &lhs, foeExporter const &rhs);

FOE_IMEX_EXPORT foeResult foeImexRegisterExporter(foeExporter exporter);
FOE_IMEX_EXPORT foeResult foeImexDeregisterExporter(foeExporter exporter);

FOE_IMEX_EXPORT void foeImexGetExporters(uint32_t *pExporterCount, foeExporter *pExporters);

struct foeExportFunctionality {
    foeResult (*onRegister)(foeExporter);
    void (*onDeregister)(foeExporter);

    inline bool operator==(foeExportFunctionality const &rhs) const noexcept {
        return onRegister == rhs.onRegister && onDeregister == rhs.onDeregister;
    }
    inline bool operator!=(foeExportFunctionality const &rhs) const noexcept {
        return !(this == &rhs);
    }
};

FOE_IMEX_EXPORT foeResult
foeRegisterExportFunctionality(foeExportFunctionality const &functionality);
FOE_IMEX_EXPORT void foeDeregisterExportFunctionality(foeExportFunctionality const &functionality);

#endif // FOE_IMEX_EXPORTERS_HPP