/*
    Copyright (C) 2021 George Cave.

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

#include <foe/ecs/id.hpp>
#include <foe/imex/export.h>
#include <foe/imex/exporter_base.hpp>

#include <system_error>

struct foeExportFunctionality {
    std::error_code (*onRegister)(foeExporterBase *);
    void (*onDeregister)(foeExporterBase *);

    inline bool operator==(foeExportFunctionality const &rhs) const noexcept {
        return onRegister == rhs.onRegister && onDeregister == rhs.onDeregister;
    }
    inline bool operator!=(foeExportFunctionality const &rhs) const noexcept {
        return !(this == &rhs);
    }
};

FOE_IMEX_EXPORT auto foeRegisterExportFunctionality(foeExportFunctionality const &functionality)
    -> std::error_code;
FOE_IMEX_EXPORT void foeDeregisterExportFunctionality(foeExportFunctionality const &functionality);

FOE_IMEX_EXPORT auto foeRegisterExporter(foeExporterBase *pExporter) -> std::error_code;
FOE_IMEX_EXPORT void foeDeregisterExporter(foeExporterBase *pExporter);

#endif // FOE_IMEX_EXPORTERS_HPP