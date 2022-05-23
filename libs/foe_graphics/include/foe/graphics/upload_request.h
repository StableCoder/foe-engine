/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef FOE_GRAPHICS_UPLOAD_REQUEST_H
#define FOE_GRAPHICS_UPLOAD_REQUEST_H

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/graphics/upload_context.h>
#include <foe/handle.h>

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

FOE_GFX_EXPORT foeResult foeSubmitUploadDataCommands(foeGfxUploadContext uploadContext,
                                                     foeGfxUploadRequest uploadRequest);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_UPLOAD_REQUEST_H