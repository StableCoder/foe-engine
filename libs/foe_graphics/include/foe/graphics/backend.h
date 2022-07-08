// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_BACKEND_H
#define FOE_GRAPHICS_BACKEND_H

#include <foe/graphics/export.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeGfxVersion {
    unsigned int major : 10;
    unsigned int minor : 10;
    unsigned int patch : 12;
};

#ifdef __cplusplus
static_assert(sizeof(struct foeGfxVersion) == 4,
              "foeGfxVersion should be 4 bytes in size (uint32_t)");
#else
_Static_assert(sizeof(struct foeGfxVersion) == 4,
               "foeGfxVersion should be 4 bytes in size (uint32_t)");
#endif

FOE_GFX_EXPORT char const *foeGfxBackendName();

FOE_GFX_EXPORT struct foeGfxVersion foeGfxBackendVersion();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_BACKEND_H