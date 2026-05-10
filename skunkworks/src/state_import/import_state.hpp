// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMPORT_STATE_HPP
#define IMPORT_STATE_HPP

#include <foe/handle.h>
#include <foe/result.h>

#include <string_view>

FOE_DEFINE_HANDLE(foeSimulation)

class foeSearchPaths;

/// Imports data set and its dependencies
foeResultSet importState(std::string_view topLevelDataSet,
                         foeSearchPaths *pSearchPaths,
                         foeSimulation *pSimulation);

#endif // IMPORT_STATE_HPP