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

std::vector<foeImporterGenerator *> generators;

} // namespace

auto foeRegisterImportGenerator(foeImporterGenerator *pGenerator) -> std::error_code {
    std::scoped_lock lock{gSync};

    for (auto const &it : generators) {
        if (it == pGenerator)
            return FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED;
    }

    // Add the generator
    generators.emplace_back(pGenerator);

    return FOE_IMEX_SUCCESS;
}

auto foeDeregisterImportGenerator(foeImporterGenerator *pGenerator) -> std::error_code {
    std::scoped_lock lock{gSync};

    for (auto it = generators.begin(); it != generators.end(); ++it) {
        if (*it != pGenerator)
            continue;

        // Found, remove it
        generators.erase(it);

        return FOE_IMEX_SUCCESS;
    }

    return FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED;
}

auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath) -> foeImporterBase * {
    std::scoped_lock lock{gSync};

    for (auto it : generators) {
        auto *pImporter = it->createImporter(group, stateDataPath);
        if (pImporter != nullptr)
            return pImporter;
    }

    return nullptr;
}
