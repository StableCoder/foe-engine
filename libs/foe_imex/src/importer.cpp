// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/importer.h>

#include "log.hpp"
#include "result.h"

#include <mutex>
#include <vector>

namespace {

std::mutex gSync;

std::vector<PFN_foeImexCreateImporter> gCreateImporterFns;

} // namespace

foeResultSet foeImexRegisterImporter(PFN_foeImexCreateImporter createImporter) {
    std::scoped_lock lock{gSync};

    for (auto const &it : gCreateImporterFns) {
        if (it == createImporter)
            return to_foeResult(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED);
    }

    // Add the generator
    gCreateImporterFns.emplace_back(createImporter);

    return to_foeResult(FOE_IMEX_SUCCESS);
}

foeResultSet foeImexDeregisterImporter(PFN_foeImexCreateImporter createImporter) {
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

auto createImporter(foeIdGroup group, char const *pPath) -> foeImexImporter {
    std::scoped_lock lock{gSync};

    foeImexImporter importer{FOE_NULL_HANDLE};
    for (auto it : gCreateImporterFns) {
        it(group, pPath, &importer);
        if (importer != FOE_NULL_HANDLE)
            break;
    }

    return importer;
}
