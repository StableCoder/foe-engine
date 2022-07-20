// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/importers.hpp>

#include "log.hpp"
#include "result.h"

#include <mutex>
#include <vector>

namespace {

std::mutex gSync;

std::vector<PFN_foeImexCreateImporter> gCreateImporterFns;

} // namespace

foeResult foeImexRegisterImporter(PFN_foeImexCreateImporter createImporter) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gCreateImporterFns) {
        if (it == createImporter)
            return to_foeResult(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED);
    }

    // Add the generator
    gCreateImporterFns.emplace_back(createImporter);

    return to_foeResult(FOE_IMEX_SUCCESS);
}

foeResult foeImexDeregisterImporter(PFN_foeImexCreateImporter createImporter) {
    std::scoped_lock lock{gSync};

    for (auto it = gCreateImporterFns.begin(); it != gCreateImporterFns.end(); ++it) {
        if (*it != createImporter)
            continue;

        // Found, remove it
        gCreateImporterFns.erase(it);

        return to_foeResult(FOE_IMEX_SUCCESS);
    }

    return to_foeResult(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED);
}

auto createImporter(foeIdGroup group, char const *pPath) -> foeImporterBase * {
    std::scoped_lock lock{gSync};

    foeImporterBase *pImporter{nullptr};
    for (auto it : gCreateImporterFns) {
        it(group, pPath, &pImporter);
        if (pImporter != nullptr)
            break;
    }

    return pImporter;
}
