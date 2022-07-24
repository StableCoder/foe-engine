// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_UPLOAD_CONTEXT_H
#define FOE_GRAPHICS_UPLOAD_CONTEXT_H

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxUploadContext)

FOE_GFX_EXPORT foeResultSet foeGfxCreateUploadContext(foeGfxSession session,
                                                      foeGfxUploadContext *pUploadContext);

FOE_GFX_EXPORT void foeGfxDestroyUploadContext(foeGfxUploadContext uploadContext);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_UPLOAD_CONTEXT_H