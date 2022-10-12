// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/importer.h>

#include "importer_functions.hpp"
#include "log.hpp"
#include "result.h"

#include <map>
#include <shared_mutex>
#include <string_view>

namespace {

std::shared_mutex gSync;

std::map<std::string_view, BinaryResourceFns> gResourceFns;
std::map<std::string_view, PFN_foeImexBinaryImportComponent> gComponentFns;

} // namespace

auto sharedLockImportFunctionality() -> std::shared_lock<std::shared_mutex> {
    return std::shared_lock<std::shared_mutex>{gSync};
}

auto getResourceFns() -> std::map<std::string_view, BinaryResourceFns> const & {
    return gResourceFns;
}

auto getComponentFns() -> std::map<std::string_view, PFN_foeImexBinaryImportComponent> const & {
    return gComponentFns;
}

extern "C" foeResultSet foeImexBinaryRegisterResourceImportFns(
    char const *pBinaryKey,
    PFN_foeImexBinaryImportResource importFn,
    PFN_foeImexBinaryCreateResource createFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(pBinaryKey);
    if (searchIt != gResourceFns.end()) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not register binary resource import/create functions for the binary key "
                "'{}' as it is already registered",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED);
    }

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_INFO,
            "Registering binary resource importe/create functions for the binary key '{}'",
            pBinaryKey);
    gResourceFns[pBinaryKey] = BinaryResourceFns{
        .importFn = importFn,
        .createFn = createFn,
    };

    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

extern "C" foeResultSet foeImexBinaryDeregisterResourceImportFns(
    char const *pBinaryKey,
    PFN_foeImexBinaryImportResource importFn,
    PFN_foeImexBinaryCreateResource createFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gResourceFns.find(pBinaryKey);
    if (searchIt == gResourceFns.end()) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not deregister binary resource import/create functions for the binary key "
                "'{}' as it is not currently registered",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED);
    }

    if (searchIt->second.importFn != importFn || searchIt->second.createFn != createFn) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not deregister binary resource import/create functions for the binary key "
                "'{}' as the functions do not match",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING);
    }

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_INFO,
            "Deregistering binary resource import/create functions for '{}'", pBinaryKey);
    gResourceFns.erase(searchIt);
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

extern "C" foeResultSet foeImexBinaryRegisterComponentImportFn(
    char const *pBinaryKey, PFN_foeImexBinaryImportComponent importFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gComponentFns.find(pBinaryKey);
    if (searchIt != gComponentFns.end()) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not register binary component import function for the binary key '{}' as it "
                "is already registered",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_ALREADY_REGISTERED);
    }

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_INFO,
            "Registering binary component import function for the binary key '{}'", pBinaryKey);
    gComponentFns[pBinaryKey] = importFn;

    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}

extern "C" foeResultSet foeImexBinaryDeregisterComponentImportFn(
    char const *pBinaryKey, PFN_foeImexBinaryImportComponent importFn) {
    std::unique_lock lock{gSync};

    auto searchIt = gComponentFns.find(pBinaryKey);
    if (searchIt == gComponentFns.end()) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not deregister binary component import function for the binary key "
                "'{}' as it is not currently registered",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_NOT_REGISTERED);
    }

    if (searchIt->second != importFn) {
        FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_ERROR,
                "Could not deregister binary resource import function for the binary key "
                "'{}' as the function does not match",
                pBinaryKey);
        return to_foeResult(FOE_IMEX_BINARY_ERROR_KEY_FUNCTIONS_NON_MATCHING);
    }

    FOE_LOG(foeImexBinary, FOE_LOG_LEVEL_INFO,
            "Deregistering binary component import function for '{}'", pBinaryKey);
    gComponentFns.erase(searchIt);
    return to_foeResult(FOE_IMEX_BINARY_SUCCESS);
}