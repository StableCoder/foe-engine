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

#include <foe/imex/importers.hpp>

#include "error_code.hpp"
#include "log.hpp"

#include <mutex>
#include <vector>

namespace {

std::mutex gSync;

std::vector<PFN_foeImexCreateImporter> gCreateImporterFns;

} // namespace

foeErrorCode foeImexRegisterImporter(PFN_foeImexCreateImporter createImporter) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gCreateImporterFns) {
        if (it == createImporter)
            return foeToErrorCode(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED);
    }

    // Add the generator
    gCreateImporterFns.emplace_back(createImporter);

    return foeToErrorCode(FOE_IMEX_SUCCESS);
}

foeErrorCode foeImexDeregisterImporter(PFN_foeImexCreateImporter createImporter) {
    std::scoped_lock lock{gSync};

    for (auto it = gCreateImporterFns.begin(); it != gCreateImporterFns.end(); ++it) {
        if (*it != createImporter)
            continue;

        // Found, remove it
        gCreateImporterFns.erase(it);

        return foeToErrorCode(FOE_IMEX_SUCCESS);
    }

    return foeToErrorCode(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED);
}

auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath) -> foeImporterBase * {
    std::scoped_lock lock{gSync};

    foeImporterBase *pImporter{nullptr};
    for (auto it : gCreateImporterFns) {
        it(group, stateDataPath.string().c_str(), &pImporter);
        if (pImporter != nullptr)
            break;
    }

    return pImporter;
}
