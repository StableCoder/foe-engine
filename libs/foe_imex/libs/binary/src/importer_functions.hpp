// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMPORTER_FUNCTIONS_HPP
#define IMPORTER_FUNCTIONS_HPP

#include <foe/imex/binary/importer.h>

#include <map>
#include <shared_mutex>

struct BinaryResourceFns {
    PFN_foeImexBinaryImportResource importFn;
};

auto sharedLockImportFunctionality() -> std::shared_lock<std::shared_mutex>;

auto getResourceFns() -> std::map<std::string_view, BinaryResourceFns> const &;

auto getComponentFns() -> std::map<std::string_view, PFN_foeImexBinaryImportComponent> const &;

#endif // IMPORTER_FUNCTIONS_HPP