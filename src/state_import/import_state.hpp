// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMPORT_STATE_HPP
#define IMPORT_STATE_HPP

#include <foe/error_code.h>

#include <string_view>

class foeSearchPaths;
struct foeSimulation;

/// Imports data set and its dependencies
foeResultSet importState(std::string_view topLevelDataSet,
                         foeSearchPaths *pSearchPaths,
                         foeSimulation **ppSimulationSet);

#endif // IMPORT_STATE_HPP