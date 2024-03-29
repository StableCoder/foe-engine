// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_TYPE_DEFS_H
#define FOE_PHYSICS_TYPE_DEFS_H

#define FOE_PHYSICS_LIBRARY_ID 1000018000

typedef enum foePhysicsStructureType {
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE = 1000018000,
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO = 1000018001,
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER = 1000018002,
    FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM = 1000018003,
    FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL = 1000018004,
} foePhysicsStructureType;

#endif // FOE_PHYSICS_TYPE_DEFS_H