// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_UPLOAD_BUFFER_H
#define FOE_GRAPHICS_UPLOAD_BUFFER_H

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/upload_context.h>
#include <foe/handle.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxUploadBuffer)

FOE_GFX_EXPORT foeResultSet foeGfxCreateUploadBuffer(foeGfxUploadContext uploadContext,
                                                     uint64_t size,
                                                     foeGfxUploadBuffer *pUploadBuffer);

FOE_GFX_EXPORT void foeGfxDestroyUploadBuffer(foeGfxUploadContext uploadContext,
                                              foeGfxUploadBuffer uploadBuffer);

FOE_GFX_EXPORT foeResultSet foeGfxMapUploadBuffer(foeGfxUploadContext uploadContext,
                                                  foeGfxUploadBuffer uploadBuffer,
                                                  void **ppData);

FOE_GFX_EXPORT void foeGfxUnmapUploadBuffer(foeGfxUploadContext uploadContext,
                                            foeGfxUploadBuffer uploadBuffer);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_UPLOAD_BUFFER_H