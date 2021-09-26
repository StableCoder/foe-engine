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

#include <foe/imex/exporters.hpp>

#include "error_code.hpp"
#include "log.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

namespace {

std::mutex mSync;

std::vector<foeExporterBase *> mRegisteredExporters;

std::vector<foeExporter> mAvailableExporters;

std::vector<foeExportFunctionality> mRegisteredFunctionality;

} // namespace

bool operator==(foeExporterVersion const &lhs, foeExporterVersion const &rhs) {
    return (lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.patch == rhs.patch);
}

bool operator!=(foeExporterVersion const &lhs, foeExporterVersion const &rhs) {
    return !(lhs == rhs);
}

bool operator==(foeExporter const &lhs, foeExporter const &rhs) {
    if (lhs.pName != rhs.pName) {
        if (lhs.pName == nullptr || rhs.pName == nullptr)
            return false;

        if (std::string_view{lhs.pName} != std::string_view{rhs.pName})
            return false;
    }

    return (lhs.version == rhs.version) && (lhs.pExportFn == rhs.pExportFn);
}

bool operator!=(foeExporter const &lhs, foeExporter const &rhs) { return !(lhs == rhs); }

auto foeImexRegisterExporter(foeExporter exporter) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto const &it : mAvailableExporters) {
        if (it == exporter) {
            return FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED;
        }
    }

    mAvailableExporters.emplace_back(exporter);

    return FOE_IMEX_SUCCESS;
}

auto foeImexDeregisterExporter(foeExporter exporter) -> std::error_code {
    std::scoped_lock lock{mSync};

    for (auto it = mAvailableExporters.begin(); it != mAvailableExporters.end(); ++it) {
        if (*it == exporter) {
            mAvailableExporters.erase(it);
            return FOE_IMEX_SUCCESS;
        }
    }

    return FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED;
}

void foeImexGetExporters(uint32_t *pExporterCount, foeExporter *pExporters) {
    std::scoped_lock lock{mSync};

    // If no array to place items is provided, then just return the number of items available
    if (pExporters == nullptr) {
        *pExporterCount = mAvailableExporters.size();
        return;
    }

    // If here, then copy a number of items into the provided memory
    uint32_t minCount =
        std::min(*pExporterCount, static_cast<uint32_t>(mAvailableExporters.size()));
    std::copy(mAvailableExporters.begin(), mAvailableExporters.begin() + minCount, pExporters);

    // Make sure to return the actual number copied
    *pExporterCount = minCount;
}

auto foeRegisterExportFunctionality(foeExportFunctionality const &functionality)
    -> std::error_code {
    std::error_code errC;
    std::scoped_lock lock{mSync};

    for (auto const &it : mRegisteredFunctionality) {
        if (it == functionality) {
            FOE_LOG(foeImex, Warning,
                    "foeRegisterExportFunctionality - Attempted to re-register functionality");
            return FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
        }
    }

    // Add the *new* functionality to any already-existing exporters
    if (functionality.onRegister) {
        for (auto const &it : mRegisteredExporters) {
            errC = functionality.onRegister(it);
            if (errC)
                goto REGISTRATION_FAILED;
        }
    }

    // Not already registered, add it
    mRegisteredFunctionality.emplace_back(functionality);

REGISTRATION_FAILED:
    if (errC && functionality.onDeregister) {
        for (auto const &it : mRegisteredExporters) {
            functionality.onDeregister(it);
        }
    }

    return errC;
}

void foeDeregisterExportFunctionality(foeExportFunctionality const &functionality) {
    std::scoped_lock lock{mSync};

    auto registration = mRegisteredFunctionality.begin();
    for (; registration != mRegisteredFunctionality.end(); ++registration) {
        if (*registration == functionality) {
            goto CONTINUE_DEREGISTRATION;
        }
    }

    FOE_LOG(foeImex, Warning,
            "foeDeregisterExportFunctionality - Attempted to deregister functionality that was "
            "never registered");
    return;

CONTINUE_DEREGISTRATION:
    if (registration->onDeregister) {
        for (auto const &it : mRegisteredExporters) {
            registration->onDeregister(it);
        }
    }
}

auto foeRegisterExporter(foeExporterBase *pExporter) -> std::error_code {
    std::error_code errC;
    std::scoped_lock lock{mSync};

    // Check to make sure not already registered
    for (auto const &it : mRegisteredExporters) {
        if (it == pExporter) {
            FOE_LOG(foeImex, Warning, "foeRegisterExporter - Attempted to re-register exporter");
            return FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED;
        }
    }

    for (auto const &it : mRegisteredFunctionality) {
        if (it.onRegister)
            errC = it.onRegister(pExporter);
        if (errC)
            goto REGISTRATION_FAILED;
    }

    mRegisteredExporters.emplace_back(pExporter);

REGISTRATION_FAILED:
    if (errC) {
        for (auto const &it : mRegisteredFunctionality) {
            if (it.onDeregister)
                it.onDeregister(pExporter);
        }
    }

    return errC;
}

void foeDeregisterExporter(foeExporterBase *pExporter) {
    std::scoped_lock lock{mSync};

    // Make sure it's actually registered, otherwise do nothing
    // Check to make sure not already registered
    auto registration = mRegisteredExporters.begin();
    for (; registration != mRegisteredExporters.end(); ++registration) {
        if (*registration == pExporter) {
            goto CONTINUE_DEREGISTER;
        }
    }

    FOE_LOG(foeImex, Error,
            "foeDeregisterExporter - Attempted to deregister exporter that was never registered");
    return;

CONTINUE_DEREGISTER:
    for (auto const &it : mRegisteredFunctionality) {
        if (it.onDeregister)
            it.onDeregister(pExporter);
    }

    mRegisteredExporters.erase(registration);
}