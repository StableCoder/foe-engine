// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_TYPE_DEFS_H
#define FOE_POSITION_TYPE_DEFS_H

#include <foe/simulation/type_defs.h>

#define FOE_POSITION_FUNCTIONALITY_ID FOE_SIMULATION_FUNCTIONALITY_ID(3)

typedef enum foePositionStructureType {
    FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL = FOE_POSITION_FUNCTIONALITY_ID,
} foePositionStructureType;

#endif // FOE_POSITION_TYPE_DEFS_H