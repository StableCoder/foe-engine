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

#ifndef FOE_GRAPHICS_BUFFER_HPP
#define FOE_GRAPHICS_BUFFER_HPP

#include <foe/error_code.h>
#include <foe/graphics/export.h>
#include <foe/graphics/upload_context.hpp>
#include <foe/handle.h>

#include <cstdint>

FOE_DEFINE_HANDLE(foeGfxUploadBuffer)

FOE_GFX_EXPORT foeResult foeGfxCreateUploadBuffer(foeGfxUploadContext uploadContext,
                                                  uint64_t size,
                                                  foeGfxUploadBuffer *pUploadBuffer);

FOE_GFX_EXPORT void foeGfxDestroyUploadBuffer(foeGfxUploadContext uploadContext,
                                              foeGfxUploadBuffer uploadBuffer);

FOE_GFX_EXPORT foeResult foeGfxMapUploadBuffer(foeGfxUploadContext uploadContext,
                                               foeGfxUploadBuffer uploadBuffer,
                                               void **ppData);

FOE_GFX_EXPORT void foeGfxUnmapUploadBuffer(foeGfxUploadContext uploadContext,
                                            foeGfxUploadBuffer uploadBuffer);

#endif // FOE_GRAPHICS_BUFFER_HPP