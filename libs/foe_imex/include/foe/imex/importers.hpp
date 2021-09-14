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

#ifndef IMPORTERS_HPP
#define IMPORTERS_HPP

#include <foe/ecs/id.hpp>
#include <foe/imex/export.h>

#include <filesystem>
#include <system_error>
#include <vector>

class foeImporterBase;

class foeImporterGenerator {
  public:
    virtual ~foeImporterGenerator() = default;

    virtual auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * = 0;
};

FOE_IMEX_EXPORT bool foeRegisterImportGenerator(foeImporterGenerator *pGenerator);
FOE_IMEX_EXPORT bool foeDeregisterImportGenerator(foeImporterGenerator *pGenerator);

struct foeImportFunctionality {
    std::error_code (*onRegister)(foeImporterGenerator *);
    void (*onDeregister)(foeImporterGenerator *);

    inline bool operator==(foeImportFunctionality const &rhs) const noexcept {
        return onRegister == rhs.onRegister && onDeregister == rhs.onDeregister;
    }
    inline bool operator!=(foeImportFunctionality const &rhs) const noexcept {
        return !(this == &rhs);
    }
};

FOE_IMEX_EXPORT auto foeRegisterImportFunctionality(foeImportFunctionality const &functionality)
    -> std::error_code;
FOE_IMEX_EXPORT void foeDeregisterImportFunctionality(foeImportFunctionality const &functionality);

FOE_IMEX_EXPORT auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
    -> foeImporterBase *;

#endif // IMPORTERS_HPP