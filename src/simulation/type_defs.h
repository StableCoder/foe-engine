// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBringupStructureType {
    // Resources
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE,
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO,
    // Systems
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
    FOE_BRINGUP_STRUCTURE_TYPE_RENDER_SYSTEM,
    // Components
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
    FOE_BRINGUP_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL,
    FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
} foeBringupStructureType;

#ifdef __cplusplus
}
#endif

#endif // TYPE_DEFS_H