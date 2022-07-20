// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/exporters.h>

#include "log.hpp"
#include "result.h"

#include <algorithm>
#include <mutex>
#include <vector>

namespace {

struct foeExporterRegistrar {
    std::mutex sync;
    std::vector<foeExporter> exporters;
    std::vector<foeExportFunctionality> functionality;
} gExporterRegistrar;

} // namespace

extern "C" bool foeCompareExporters(foeExporter const *lhs, foeExporter const *rhs) {
    if (lhs->pName != rhs->pName) {
        if (lhs->pName == nullptr || rhs->pName == nullptr)
            return false;

        if (std::string_view{lhs->pName} != std::string_view{rhs->pName})
            return false;
    }

    return lhs->version == rhs->version && (lhs->pExportFn == rhs->pExportFn);
}

extern "C" foeResult foeImexRegisterExporter(foeExporter exporter) {
    std::scoped_lock lock{gExporterRegistrar.sync};

    for (auto const &it : gExporterRegistrar.exporters) {
        if (foeCompareExporters(&it, &exporter)) {
            return to_foeResult(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED);
        }
    }

    foeResult result = to_foeResult(FOE_IMEX_SUCCESS);
    for (auto const &it : gExporterRegistrar.functionality) {
        if (it.onRegister)
            result = it.onRegister(exporter);
        if (result.value != FOE_SUCCESS)
            goto REGISTRATION_FAILED;
    }

    gExporterRegistrar.exporters.emplace_back(exporter);

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        for (auto const &it : gExporterRegistrar.functionality) {
            if (it.onDeregister)
                it.onDeregister(exporter);
        }
    }

    return result;
}

extern "C" foeResult foeImexDeregisterExporter(foeExporter exporter) {
    std::scoped_lock lock{gExporterRegistrar.sync};

    for (auto it = gExporterRegistrar.exporters.begin(); it != gExporterRegistrar.exporters.end();
         ++it) {
        if (foeCompareExporters(&(*it), &exporter)) {
            gExporterRegistrar.exporters.erase(it);
            goto CONTINUE_DEREGISTER;
        }
    }

    FOE_LOG(foeImex, Error,
            "foeDeregisterExporter - Attempted to deregister exporter that was not registered");
    return to_foeResult(FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED);

CONTINUE_DEREGISTER:
    for (auto const &it : gExporterRegistrar.functionality) {
        if (it.onDeregister)
            it.onDeregister(exporter);
    }

    return to_foeResult(FOE_IMEX_SUCCESS);
}

extern "C" void foeImexGetExporters(uint32_t *pExporterCount, foeExporter *pExporters) {
    std::scoped_lock lock{gExporterRegistrar.sync};

    // If no array to place items is provided, then just return the number of items available
    if (pExporters == nullptr) {
        *pExporterCount = gExporterRegistrar.exporters.size();
        return;
    }

    // If here, then copy a number of items into the provided memory
    uint32_t minCount =
        std::min(*pExporterCount, static_cast<uint32_t>(gExporterRegistrar.exporters.size()));
    std::copy(gExporterRegistrar.exporters.begin(), gExporterRegistrar.exporters.begin() + minCount,
              pExporters);

    // Make sure to return the actual number copied
    *pExporterCount = minCount;
}

extern "C" foeResult foeRegisterExportFunctionality(foeExportFunctionality const *functionality) {
    foeResult result = {.value = FOE_SUCCESS, .toString = NULL};
    std::scoped_lock lock{gExporterRegistrar.sync};

    for (auto const &it : gExporterRegistrar.functionality) {
        if (it.onRegister == functionality->onRegister &&
            it.onDeregister == functionality->onDeregister) {
            FOE_LOG(foeImex, Warning,
                    "foeRegisterExportFunctionality - Attempted to re-register functionality");
            return to_foeResult(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED);
        }
    }

    // Add the *new* functionality to any already-existing exporters
    if (functionality->onRegister) {
        for (auto const &it : gExporterRegistrar.exporters) {
            result = functionality->onRegister(it);
            if (result.value != FOE_SUCCESS)
                goto REGISTRATION_FAILED;
        }
    }

    // Not already registered, add it
    gExporterRegistrar.functionality.emplace_back(*functionality);

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS && functionality->onDeregister) {
        for (auto const &it : gExporterRegistrar.exporters) {
            functionality->onDeregister(it);
        }
    }

    return result;
}

extern "C" void foeDeregisterExportFunctionality(foeExportFunctionality const *functionality) {
    std::scoped_lock lock{gExporterRegistrar.sync};

    for (auto it = gExporterRegistrar.functionality.begin();
         it != gExporterRegistrar.functionality.end(); ++it) {
        if (it->onRegister == functionality->onRegister &&
            it->onDeregister == functionality->onDeregister) {
            gExporterRegistrar.functionality.erase(it);
            goto CONTINUE_DEREGISTRATION;
        }
    }

    FOE_LOG(foeImex, Warning,
            "foeDeregisterExportFunctionality - Attempted to deregister functionality that was "
            "never registered");
    return;

CONTINUE_DEREGISTRATION:
    if (functionality->onDeregister) {
        for (auto const &it : gExporterRegistrar.exporters) {
            functionality->onDeregister(it);
        }
    }
}