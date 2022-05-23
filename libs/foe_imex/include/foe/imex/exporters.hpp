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

#ifndef FOE_IMEX_EXPORTERS_HPP
#define FOE_IMEX_EXPORTERS_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/export.h>

#include <filesystem>

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
    foeResult (*pExportFn)(std::filesystem::path, foeSimulation *);
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