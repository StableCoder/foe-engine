// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_MESH_H
#define FOE_GRAPHICS_MESH_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxMesh)

FOE_GFX_EXPORT uint32_t foeGfxGetMeshIndices(foeGfxMesh mesh);

FOE_GFX_EXPORT void foeGfxDestroyMesh(foeGfxSession session, foeGfxMesh mesh);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_MESH_H