// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_IMAGE_CREATE_INFO_H
#define FOE_GRAPHICS_RESOURCE_IMAGE_CREATE_INFO_H

#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeImageCreateInfo {
    char const *pFile;
} foeImageCreateInfo;

FOE_GFX_RES_EXPORT void foeCleanup_foeImageCreateInfo(foeImageCreateInfo *pCreateInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_IMAGE_CREATE_INFO_H