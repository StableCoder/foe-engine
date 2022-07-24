// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_UPLOAD_REQUEST_H
#define FOE_GRAPHICS_UPLOAD_REQUEST_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/graphics/upload_context.h>
#include <foe/handle.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxUploadRequest)

typedef enum foeGfxUploadRequestStatus {
    FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE = 0,
    FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE,
    FOE_GFX_UPLOAD_REQUEST_STATUS_DEVICE_LOST,
} foeGfxUploadRequestStatus;

/**
 * @brief Returns the status of an upload request
 * @param uploadRequest Request to get the completion status for
 * @return VK_SUCCESS if the request hasn't been submitted, or if it has and has finished. An
 * appropriate value otherwise.
 */
FOE_GFX_EXPORT foeGfxUploadRequestStatus
foeGfxGetUploadRequestStatus(foeGfxUploadRequest uploadRequest);

FOE_GFX_EXPORT void foeGfxDestroyUploadRequest(foeGfxUploadContext uploadContext,
                                               foeGfxUploadRequest uploadRequest);

FOE_GFX_EXPORT foeResultSet foeSubmitUploadDataCommands(foeGfxUploadContext uploadContext,
                                                        foeGfxUploadRequest uploadRequest);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_UPLOAD_REQUEST_H